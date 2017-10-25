// originally from Wade Chien
// https://ez.analog.com/message/322762-re-ad9910-drg-mode-in-amplitude-sweep-triangle-waveform-output-full-rang-different-from-single-tone?commentID=322762&et=watches.email.thread#comment-322762
// Adapted with permission

/*
 * ad9910.c
 *
 * Created: 24/10/2017 1:06:15 PM
 *  Author: bob
 */ 
#include <spi_basic.h>
#include <stdio.h>
#include "timeutils.h"
#include "ad9910.h"
#include "math.h"

uint16_t dds_singletone_amp  = 16383;	//preset maximum output amplitude(14b DAC)
uint32_t dds_singletone_freq = 1000;	//preset frequency 1000Hz
uint16_t dds_digr_duty = 50;   			//preset duty 50%
uint32_t dds_digr_freq = 1000; 			//preset frequency1000Hz


uint8_t spiRxBuf[SPI1_RX_BUF_SIZ]={0};
//volatile uint32_t ad9910_delay = 0;
//void HAL_Delay(uint32_t n) { ad9910_delay = n*1000000; while(ad9910_delay > 0) { ad9910_delay--;	} }

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Function : AD9910_Reset
//Description : N/a
//Arguments: N/a
// currently no reset line implemented in freqref
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_reset(void)
{
#if 0
	AD9910_MRST_L;
	 delay_ms(1);	//delay 1ms
	AD9910_MRST_H;
	 delay_ms(1);	//delay 1ms
	AD9910_MRST_L;
	 delay_ms(1);	//delay 1ms
#endif
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Function: AD9910_init
//Description : initialize the AD9910
//Arguments : N/a
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_init(void)
{
	//AD9910 configuration
	AD9910_reset();
	CS_DDS_set_level(high);								//de-select AD9910, disable SPI transmission
//	CS_DDS_set_level(low);
									//ready for SPI communication
	//profile 0 selected
	AD9910_SET_PROFILE(0);									//profile 0 has been selected
	
	//default value of single tone mode
	AD9910_CFR1_config(DDS_SINE,0,0,bWRITE);				//set CRF1 to single tone and sine wave output
	AD9910_CFR2_config(bWRITE, DISABLE_DRG);				//disable DRG for single tone output
	AD9910_CFR3_config(PLL_MULTIPLY, 1, bWRITE);			//fxtal=25MHz, N=40, fsysclk=25MHz*40=1GHz, enable PLL
	//AD9910_AuxDAC_config(219, bWRITE);					//DAC code=219, Iout=(86.4/10k)*(1+218/96)=28.35mA (=20.07mA/0.707)
	AD9910_AuxDAC_config(127, bWRITE);						//DAC code=127, Iout=(86.4/10k)*(1+127/96)=20.07mA
	//AD9910_FTW_config(4295, bWRITE);						//default 1kHz (sysclk=1GHz), ftw=round((1kHz/1GHz)*4294967295)
	//AD9910_POW_config(0x0000, bWRITE);					//phase shift = 0(default)
	//AD9910_ASF_config(0x3FFF, bWRITE);					//amplitude scale factor= 0x3FFF, full scale-14 bit DAC(default)
	AD9910_SingleTone_PF0(0x3FFF, 0x0000, 4295, bWRITE);	//ftw=4295=1kHz
	AD9910_IO_UPDATE();										//update IO buffer to register
	AD9910_SET_PROFILE(0);									//Singel-tone profile 0 has been selected
	
	//---read back register---
	//AD9910_CFR1_config(bREAD);
	//AD9910_CFR2_config(bREAD);
	AD9910_CFR3_config(0, 0, bREAD);			// zzz 
	//AD9910_AuxDAC_config(0, bREAD);
	//AD9910_FTW_config(0, bREAD);
	//AD9910_POW_config(0, bREAD);
	//AD9910_ASF_config(0, bREAD);
	//AD9910_SingleTone_PF0(0, 0, 0, bREAD);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Function: AD9910_single_tone
//Description : single tone mode setting
//Arguments :
//	amplitude	: amplitude scale factor 0~16383(0x3FFF)
//	phase		: phase shift(0~0xFFFF) = 2pi*(phase/65536)
//	frequency	: frequency(Hz) by uint32_t, 0<f<40MHz(0x2625A00)
// 	set_prof	: select destination profile (0~7)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_single_tone(uint16_t amplitude, uint16_t phase_offset, uint32_t frequency, uint8_t set_prof)
{
	if(amplitude > 0x3FFF) return;									//amplitude must less than 0x3FFF(14-bit DAC)
	if(phase_offset > 0xFFFF) return;								//phase shift(16bit wide)
	if(frequency > 200000000) return;								//frequency must less than 100MHz
	
	double ftw_word;
	double freq_ratio;
	uint32_t ftw;
	
	//calculate frequency tuning word
	freq_ratio = (((double) frequency * 1.0)/AD9910_SYSCLK);
	ftw_word = round(freq_ratio*0xFFFFFFFF);
	ftw = (uint32_t)ftw_word;
	
	
	AD9910_CFR1_config(DDS_SINE,0,0,bWRITE);						//config CFR1 for single tone(default=sinewave output)
	AD9910_CFR2_config(DISABLE_DRG, bWRITE);						//disable DRG for single tone output
	//AD9910_FTW_config(ftw, bWRITE);
	//AD9910_ASF_config(amp_scale, bWRITE);
	switch(set_prof)
	{
		case 0x00: AD9910_SingleTone_PF0(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 0 value
		case 0x01: AD9910_SingleTone_PF1(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 1 value
		case 0x02: AD9910_SingleTone_PF2(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 2 value
		case 0x03: AD9910_SingleTone_PF3(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 3 value
		case 0x04: AD9910_SingleTone_PF4(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 4 value
		case 0x05: AD9910_SingleTone_PF5(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 5 value
		case 0x06: AD9910_SingleTone_PF6(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 6 value
		case 0x07: AD9910_SingleTone_PF7(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 7 value
		default	: AD9910_SingleTone_PF0(amplitude, phase_offset, ftw, bWRITE);	break;	//write single-tone profile 0 value
	}
	AD9910_IO_UPDATE();												//update IO buffer to register
	AD9910_SET_PROFILE(set_prof);									//select profile(0~7)
	
	
	//---read back register---
	//AD9910_SingleTone_PF0(0, 0, 0, bREAD);						//read back profile 0 register
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Function: AD9910_triangle_wave
// Description : enable digital ramp generator for triangle waveform(amplitude sweep)
// parameters :
//	duty: duty cycle by type uint8_t, value: 0<duty(%)<100
//	freq: frequency by type uint32_t, value: 0<f(Hz)<10000000
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_triangle_wave(uint32_t freq, uint8_t duty)
{
	if((freq <= 0x000001)||(freq >= 0x989680)) return; 				//frequency should be set less than 10MHz
	if((duty <= 0x00)||(duty >= 0x64)) return;						//duty cycle over 100% or less than 0 doesn't make sense (1~99 only)
	
	//calculate triangle ramp_up and ramp_down time
	double duty_cycle 		= (((double)duty) / 100.0);				//1%~99%
	double period 			= (1.0 / ((double)freq));				//period = 1/ freq
	double ramp_up_time		= (duty_cycle * period);				//Tup = D*T
	double ramp_down_time 	= (1.0 - duty_cycle) * period;			//Tdown = (1-D)*T
	double ddsclk_period 	= (1.0 / ((double)DDSCLK));				//DDSCLK = 0.25*SYSCLK
	
	volatile uint32_t total_step_count = (uint32_t)round(period/ddsclk_period);
	total_step_count = (total_step_count >=0x3FFF) ? 0x3FFF : total_step_count;					//limit total steps up to 0x3FFF
	
	volatile uint32_t pos_step_count =(uint32_t)round(((double)total_step_count)*duty_cycle);	//step resolution limited by dds clock(250MHZ)
	volatile uint32_t neg_step_count =total_step_count - pos_step_count;						//step resolution limited by dds clock(250MHZ)
	//pos_step_count = (pos_step_count >= 0x3FFF) ? 0x3FFF : pos_step_count;					//also limited by DAC resolution, 14bit, maximum 0x3FFF
	//neg_step_count = (neg_step_count >= 0x3FFF) ? 0x3FFF : neg_step_count;					//also limited by DAC resolution, 14bit, maximum 0x3FFF

	//set upper and lower limit
	volatile uint32_t upper_limit = 0xffffffff;													//set to maximum(32bit) when triangle mode
	volatile uint32_t lower_limit = 0x00000000;													//set to minimum(32bit) when triangle mode

	//calculate pos/neg step size (according to frequency and duty)
	volatile uint32_t pos_step_size = (uint32_t)round((double)DAC_MAX_RESOLUTION/(double)pos_step_count); 	//step size from 1 to 16383(maximum)
	volatile uint32_t neg_step_size = (uint32_t)round((double)DAC_MAX_RESOLUTION/(double)neg_step_count);	//step size from 1 to 16383(maximum)
	pos_step_size = pos_step_size<<18;																		//data valid only [31:18](MSB first)
	neg_step_size = neg_step_size<<18;																		//data valid only [31:18](MSB first)


	//calculate pos/neg step rate (according to frequency and duty)
	volatile double pos_delta_t = (double)ramp_up_time/(double)pos_step_count;			//positive ramp step interval time
	volatile double neg_delta_t = (double)ramp_down_time/(double)neg_step_count;		//negative ramp step interval time
	volatile uint16_t pos_ramp_rate = (uint16_t)(round(pos_delta_t / ddsclk_period));	//for how many dds_clock counts per (+)interval
	volatile uint16_t neg_ramp_rate = (uint16_t)(round(neg_delta_t / ddsclk_period));	//for how many dds_clock counts per (-)interval


	//AD9910_AuxDAC_config(127, bWRITE);												//DAC code=127, Iout=(86.4/10k)*(1+127/96)=20.07mA
	AD9910_single_tone(DAC_MAX_RESOLUTION, 0,0,SET_PROFILE0);							//set FTW=0 to stop sinewaves(DC output)
	AD9910_CFR1_config(DDS_COSINE,0,0,bWRITE);											//set dds output type to cosine
	//AD9910_CFR1_config(DDS_SINE,0,0,bWRITE);											//set dds output type to sine
	AD9910_DIGR_LIMIT(upper_limit, lower_limit, bWRITE);								//write upper/lower limit
	AD9910_DIGR_STEPSIZE(pos_step_size, neg_step_size, bWRITE);							//write positive/negative ramp step size, [31:18]
	AD9910_DIGR_RATE(pos_ramp_rate, neg_ramp_rate, bWRITE);								//write positive/negative ramp rate
	AD9910_CFR2_config(ENABLE_DRG_TRI, bWRITE);											//(1)enable DRG:CFR2[19]=1; (2)no-dwell high/low:CFR2[18]=1,CFR2[17]=1; (3)set sweep destination(amplitude):CFR2[21]=1, CFR2[20]=0
	AD9910_IO_UPDATE();																	//update IO buffer to register

}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Function: AD9910_CFR1_config
//Description : AD9910 CFR2 register configuration
//Arguments :
//		sinecos: 1=sine wave;0=cosine wave(default)
//		drg_acc_clr: 1=asynchronous clear digital ramp accumulator
//		drg_pha_clr: 1=asynchronous phase accumulator
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_CFR1_config(uint8_t sinecos, uint8_t drg_acc_clr, uint8_t pha_acc_clr, uint8_t bReadWrite)
{
	REG_TypeDef CFR1_cfg;

	CFR1_cfg.RegAddr = (bReadWrite == 1) ? (CFR1_REG|CMD_READ):(CFR1_REG|CMD_WRITE);
	CFR1_cfg.Data[3] = (SDIO_IPT << 1) ;										//configure SDIO as input only(3-wire SPI), and MSB first
	CFR1_cfg.Data[2] = 0x00 | (drg_acc_clr <<6) | (pha_acc_clr <<5);
	CFR1_cfg.Data[1] = ((sinecos==1)?DDS_SINE:DDS_COSINE);						//disable sinc filter & output waveform(sine/cosine)
	//CFR1_cfg.Data[1] = (INVSIC << 6) | ((sinecos==1)?DDS_SINE:DDS_COSINE);	//enable sinc filter may experience gain attenuation for 0.7x
	CFR1_cfg.Data[0] = 0;
	CFR1_cfg.DataSize = 5;
	AD9910_ReadWrite_REG(&CFR1_cfg, spiRxBuf, CFR1_cfg.DataSize);
}

//toggle digital ramp generator accumulator asynchronous clear
void AD9910_drg_acc_clr(void)
{
	AD9910_CFR1_config(DDS_COSINE,1,0,bWRITE);	//ascychronous clear
	AD9910_IO_UPDATE();
	AD9910_CFR1_config(DDS_COSINE,0,0,bWRITE);	//default clear
	AD9910_IO_UPDATE();
}
//toggle phase accumulator asynchronous clear
void AD9910_pha_acc_clr(void)
{
	AD9910_CFR1_config(DDS_COSINE,0,1,bWRITE);	//ascychronous clear
	AD9910_IO_UPDATE();
	AD9910_CFR1_config(DDS_COSINE,0,0,bWRITE);	//default clear
	AD9910_IO_UPDATE();
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_CFR2_config
//	Description : AD9910 CFR2 register configuration
//	Arguments :
//		bReadWrite: read=1,write=0
//		drg_function :	DISABLE_DRG			(0x00)
//						ENABLE_DRG_TRI		(0x01)
//						ENABLE_DRG_SQU		(0x02)
//						ENABLE_DRG_SAW_UP	(0x03)
//						ENABLE_DRG_SAW_DOWN	(0x04)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_CFR2_config(uint8_t drg_function, uint8_t bReadWrite)
{
	REG_TypeDef CFR2_cfg;
	CFR2_cfg.RegAddr = (bReadWrite == 1) ? (CFR2_REG|CMD_READ):(CFR2_REG|CMD_WRITE);
	CFR2_cfg.Data[3] = 0;
	CFR2_cfg.Data[2] = 0;
	switch(drg_function)
	{
		case DISABLE_DRG:
		{
			CFR2_cfg.Data[1] = 0x00 | (READ_FTW << 0) | (SYNCCLK_ENA << 6);	//enable sync clock output & read ftw from DDS phase accumulator
			break;
		}
		case ENABLE_DRG_TRI:
		{
			CFR2_cfg.Data[1] =  0x00
			| (READ_FTW 		<< 0) 						//read ftw from DDS phase accumulator
			| (DIGR_NDWELL_L 	<< 1)						//set No-Dwell low
			| (DIGR_NDWELL_H 	<< 2)						//set No-Dwell high
			| (DIGR_ENABLE 	<< 3)						//enable DRG function
			| (DIGR_DES_BIT0	<< 4)						//set DRG desitination as "amplitude"
			| (DIGR_DES_BIT1	<< 5)						//set DRG desitination as "amplitude"
			| (SYNCCLK_ENA 	<< 6);						//enable sync clock output
			break;
		}
		case ENABLE_DRG_SAW_UP:
		{
			//TBD
			break;
		}
		case ENABLE_DRG_SAW_DOWN:
		{
			//TBD
			break;
		}
		case ENABLE_DRG_SQU:
		{
			//TBD
			break;
		}
		default:
		{
			CFR2_cfg.Data[1] = (READ_FTW << 0) | (SYNCCLK_ENA << 6);		//enable sync clock output & read ftw from DDS phase accumulator
			break;
		}
	}

	CFR2_cfg.Data[0] = 1;													//enable amplitude scale set by selected profile
	CFR2_cfg.DataSize = 5;
	AD9910_ReadWrite_REG(&CFR2_cfg, spiRxBuf, CFR2_cfg.DataSize);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_CFR3_config
//	Description : AD9910 CFR3 register configuration
//	Arguments :
//		pll_multiplier: N=12~127
//		pll_enable: enable PLL
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_CFR3_config(uint8_t pll_multipier, uint8_t bPLLENAB, uint8_t bReadWrite)
{
	REG_TypeDef CFR3_cfg;
	
	CFR3_cfg.RegAddr = (bReadWrite == 1) ? (CFR3_REG|CMD_READ):(CFR3_REG|CMD_WRITE);
	CFR3_cfg.Data[3] = (pll_multipier << 1);			//pll_multipier(N)=12 to 127(PLL multiplier) ex: fxtal=25MHz, N=25, fsysclk=25MHz*25=625MHz
	//CFR3_cfg.Data[2] = PLL_ENA;
	CFR3_cfg.Data[2] = ((bPLLENAB == 1) ? PLL_ENA : 0);
	CFR3_cfg.Data[1] = (ICP_387UA << 3);
	CFR3_cfg.Data[0] = VCO5 | (RCLKOUT_LOW << 4);
	CFR3_cfg.DataSize = 5;
	AD9910_ReadWrite_REG(&CFR3_cfg, spiRxBuf, CFR3_cfg.DataSize);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_AuxDAC_config
//	Description : AD9910 Auxiliary DAC control register configuration
//	Arguments :
//		DACCode: use 0~127 value to set full scale DAC current
//
//			Iout = (86.4/RSET)*(1 + DACCode/96)
//			ex. Code=127(0x7F), Iout=(86.4/10k)*(1+127/96)=20.07mA
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_AuxDAC_config(uint8_t DACCode, uint8_t bReadWrite)
{
	REG_TypeDef AuxDAC_cfg;

	AuxDAC_cfg.RegAddr = (bReadWrite == 1) ? (AUX_DAC_CTRL_REG|CMD_READ):(AUX_DAC_CTRL_REG|CMD_WRITE);
	AuxDAC_cfg.Data[3] = DACCode;				//default=0x7F
	AuxDAC_cfg.Data[2] = 0;						//open
	AuxDAC_cfg.Data[1] = 0;						//open
	AuxDAC_cfg.Data[0] = 0;						//open
	AuxDAC_cfg.DataSize = 5;
	AD9910_ReadWrite_REG(&AuxDAC_cfg, spiRxBuf, AuxDAC_cfg.DataSize);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_IOupdate_config
//	Description : AD9910 I/O update register configuration
//	Arguments : (Note: Effective only if CFR2[23]=1 (internal IO update))
//		bReadWrite: read=1,write=0
//		automatic update rate=0xFFFFFFFF(default value)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_IOupdate_config(uint8_t bReadWrite)
{
	uint32_t io_update_rate = 0xFFFFFFFF;
	REG_TypeDef IOupdate_cfg;
	
	IOupdate_cfg.RegAddr = (bReadWrite == 1) ? (IO_UPDATE_RATE_REG|CMD_READ):(IO_UPDATE_RATE_REG|CMD_WRITE);
	IOupdate_cfg.Data[3] = io_update_rate;
	IOupdate_cfg.Data[2] = (io_update_rate >> 8);
	IOupdate_cfg.Data[1] = (io_update_rate >> 16);
	IOupdate_cfg.Data[0] = (io_update_rate >> 24);
	IOupdate_cfg.DataSize = 5;
	AD9910_ReadWrite_REG(&IOupdate_cfg, spiRxBuf, IOupdate_cfg.DataSize);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_FTW_config
//	Description : AD9910 frequency tuning word register configuration
//	Arguments :
//		ftword : frequency tuning word
//		ex: Fout = (ftword/2^32) * Fsysclk
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_FTW_config(uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef FTW_cfg;

	FTW_cfg.RegAddr = (bReadWrite == 1) ? (FTW_REG|CMD_READ):(FTW_REG|CMD_WRITE);
	FTW_cfg.Data[3] = (uint8_t)ftword;
	FTW_cfg.Data[2] = (uint8_t)(ftword >> 8);
	FTW_cfg.Data[1] = (uint8_t)(ftword >> 16);
	FTW_cfg.Data[0] = (uint8_t)(ftword >> 24);
	FTW_cfg.DataSize = 5;
	AD9910_ReadWrite_REG(&FTW_cfg, spiRxBuf, FTW_cfg.DataSize);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_POW_config
//	Description : AD9910 phase offset word register configuration
//	Arguments :
//		phase: phase offset
//			0 -> degree
//			(2^16-1) -> 360 degree
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_POW_config(uint16_t phase, uint8_t bReadWrite)
{
	REG_TypeDef POW_cfg;

	POW_cfg.RegAddr = (bReadWrite == 1) ? (POW_REG|CMD_READ):(POW_REG|CMD_WRITE);
	POW_cfg.Data[1] = phase;				//[7:0]
	POW_cfg.Data[0] = (phase >> 8);			//[15:8]
	POW_cfg.DataSize = 3;
	AD9910_ReadWrite_REG(&POW_cfg, spiRxBuf, POW_cfg.DataSize);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_ASF_config
//	Description : AD9910 amplitude scale factor register configuration
//	Arguments :
//		amp_scale:
//		RxBuf:
//		bReadWrite: read=1,write=0
//	Note: only effective if OSK enable or/and  automatic OSK enabled.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_ASF_config(uint16_t amp_scale, uint8_t bReadWrite)
{
	REG_TypeDef ASF_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	ASF_cfg.RegAddr = (bReadWrite == 1) ? (ASF_REG|CMD_READ):(ASF_REG|CMD_WRITE);
	ASF_cfg.Data[3] = (uint8_t)(amp_scale << 2);	//[5:0]
	ASF_cfg.Data[2] = (uint8_t)(amp_scale >> 6);	//[13:6]
	ASF_cfg.Data[1] = 0;							//effective only if CFR1[9:8]=11b
	ASF_cfg.Data[0] = 0;							//effective only if CFR1[9:8]=11b
	ASF_cfg.DataSize = 5;
	AD9910_ReadWrite_REG(&ASF_cfg, spiRxBuf, ASF_cfg.DataSize);
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_SPI_PROFILE0~7
//	Description : AD9910 serial interface profile 0 configuration
//	Arguments :
//		amp_scale:
//		phase_offset:
//		ftword:
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_SingleTone_PF0(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef SPROFILE0_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	SPROFILE0_cfg.RegAddr = (bReadWrite == 1) ? (PROFILE0_REG|CMD_READ):(PROFILE0_REG|CMD_WRITE);
	SPROFILE0_cfg.Data[7] = (uint8_t)(ftword >> 0);
	SPROFILE0_cfg.Data[6] = (uint8_t)(ftword >> 8);
	SPROFILE0_cfg.Data[5] = (uint8_t)(ftword >> 16);
	SPROFILE0_cfg.Data[4] = (uint8_t)(ftword >> 24);
	SPROFILE0_cfg.Data[3] = (uint8_t)(phase_offset >> 0);
	SPROFILE0_cfg.Data[2] = (uint8_t)(phase_offset >> 8);
	SPROFILE0_cfg.Data[1] = (uint8_t)(amp_scale >> 0);
	SPROFILE0_cfg.Data[0] = (uint8_t)(amp_scale >> 8);
	SPROFILE0_cfg.DataSize = 9;
	AD9910_ReadWrite_REG(&SPROFILE0_cfg, spiRxBuf, SPROFILE0_cfg.DataSize);
}
void AD9910_SingleTone_PF1(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef SPROFILE1_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	SPROFILE1_cfg.RegAddr = (bReadWrite == 1) ? (PROFILE1_REG|CMD_READ):(PROFILE1_REG|CMD_WRITE);
	SPROFILE1_cfg.Data[7] = (uint8_t)(ftword >> 0);
	SPROFILE1_cfg.Data[6] = (uint8_t)(ftword >> 8);
	SPROFILE1_cfg.Data[5] = (uint8_t)(ftword >> 16);
	SPROFILE1_cfg.Data[4] = (uint8_t)(ftword >> 24);
	SPROFILE1_cfg.Data[3] = (uint8_t)(phase_offset >> 0);
	SPROFILE1_cfg.Data[2] = (uint8_t)(phase_offset >> 8);
	SPROFILE1_cfg.Data[1] = (uint8_t)(amp_scale >> 0);
	SPROFILE1_cfg.Data[0] = (uint8_t)(amp_scale >> 8);
	SPROFILE1_cfg.DataSize = 9;
	AD9910_ReadWrite_REG(&SPROFILE1_cfg, spiRxBuf, SPROFILE1_cfg.DataSize);
}
void AD9910_SingleTone_PF2(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef SPROFILE2_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	SPROFILE2_cfg.RegAddr = (bReadWrite == 1) ? (PROFILE2_REG|CMD_READ):(PROFILE2_REG|CMD_WRITE);
	SPROFILE2_cfg.Data[7] = (uint8_t)(ftword >> 0);
	SPROFILE2_cfg.Data[6] = (uint8_t)(ftword >> 8);
	SPROFILE2_cfg.Data[5] = (uint8_t)(ftword >> 16);
	SPROFILE2_cfg.Data[4] = (uint8_t)(ftword >> 24);
	SPROFILE2_cfg.Data[3] = (uint8_t)(phase_offset >> 0);
	SPROFILE2_cfg.Data[2] = (uint8_t)(phase_offset >> 8);
	SPROFILE2_cfg.Data[1] = (uint8_t)(amp_scale >> 0);
	SPROFILE2_cfg.Data[0] = (uint8_t)(amp_scale >> 8);
	SPROFILE2_cfg.DataSize = 9;
	AD9910_ReadWrite_REG(&SPROFILE2_cfg, spiRxBuf, SPROFILE2_cfg.DataSize);
}
void AD9910_SingleTone_PF3(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef SPROFILE3_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	SPROFILE3_cfg.RegAddr = (bReadWrite == 1) ? (PROFILE3_REG|CMD_READ):(PROFILE3_REG|CMD_WRITE);
	SPROFILE3_cfg.Data[7] = (uint8_t)(ftword >> 0);
	SPROFILE3_cfg.Data[6] = (uint8_t)(ftword >> 8);
	SPROFILE3_cfg.Data[5] = (uint8_t)(ftword >> 16);
	SPROFILE3_cfg.Data[4] = (uint8_t)(ftword >> 24);
	SPROFILE3_cfg.Data[3] = (uint8_t)(phase_offset >> 0);
	SPROFILE3_cfg.Data[2] = (uint8_t)(phase_offset >> 8);
	SPROFILE3_cfg.Data[1] = (uint8_t)(amp_scale >> 0);
	SPROFILE3_cfg.Data[0] = (uint8_t)(amp_scale >> 8);
	SPROFILE3_cfg.DataSize = 9;
	AD9910_ReadWrite_REG(&SPROFILE3_cfg, spiRxBuf, SPROFILE3_cfg.DataSize);
}
void AD9910_SingleTone_PF4(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef SPROFILE4_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	SPROFILE4_cfg.RegAddr = (bReadWrite == 1) ? (PROFILE4_REG|CMD_READ):(PROFILE4_REG|CMD_WRITE);
	SPROFILE4_cfg.Data[7] = (uint8_t)(ftword >> 0);
	SPROFILE4_cfg.Data[6] = (uint8_t)(ftword >> 8);
	SPROFILE4_cfg.Data[5] = (uint8_t)(ftword >> 16);
	SPROFILE4_cfg.Data[4] = (uint8_t)(ftword >> 24);
	SPROFILE4_cfg.Data[3] = (uint8_t)(phase_offset >> 0);
	SPROFILE4_cfg.Data[2] = (uint8_t)(phase_offset >> 8);
	SPROFILE4_cfg.Data[1] = (uint8_t)(amp_scale >> 0);
	SPROFILE4_cfg.Data[0] = (uint8_t)(amp_scale >> 8);
	SPROFILE4_cfg.DataSize = 9;
	AD9910_ReadWrite_REG(&SPROFILE4_cfg, spiRxBuf, SPROFILE4_cfg.DataSize);
}
void AD9910_SingleTone_PF5(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef SPROFILE5_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	SPROFILE5_cfg.RegAddr = (bReadWrite == 1) ? (PROFILE5_REG|CMD_READ):(PROFILE5_REG|CMD_WRITE);
	SPROFILE5_cfg.Data[7] = (uint8_t)(ftword >> 0);
	SPROFILE5_cfg.Data[6] = (uint8_t)(ftword >> 8);
	SPROFILE5_cfg.Data[5] = (uint8_t)(ftword >> 16);
	SPROFILE5_cfg.Data[4] = (uint8_t)(ftword >> 24);
	SPROFILE5_cfg.Data[3] = (uint8_t)(phase_offset >> 0);
	SPROFILE5_cfg.Data[2] = (uint8_t)(phase_offset >> 8);
	SPROFILE5_cfg.Data[1] = (uint8_t)(amp_scale >> 0);
	SPROFILE5_cfg.Data[0] = (uint8_t)(amp_scale >> 8);
	SPROFILE5_cfg.DataSize = 9;
	AD9910_ReadWrite_REG(&SPROFILE5_cfg, spiRxBuf, SPROFILE5_cfg.DataSize);
}
void AD9910_SingleTone_PF6(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef SPROFILE6_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	SPROFILE6_cfg.RegAddr = (bReadWrite == 1) ? (PROFILE6_REG|CMD_READ):(PROFILE6_REG|CMD_WRITE);
	SPROFILE6_cfg.Data[7] = (uint8_t)(ftword >> 0);
	SPROFILE6_cfg.Data[6] = (uint8_t)(ftword >> 8);
	SPROFILE6_cfg.Data[5] = (uint8_t)(ftword >> 16);
	SPROFILE6_cfg.Data[4] = (uint8_t)(ftword >> 24);
	SPROFILE6_cfg.Data[3] = (uint8_t)(phase_offset >> 0);
	SPROFILE6_cfg.Data[2] = (uint8_t)(phase_offset >> 8);
	SPROFILE6_cfg.Data[1] = (uint8_t)(amp_scale >> 0);
	SPROFILE6_cfg.Data[0] = (uint8_t)(amp_scale >> 8);
	SPROFILE6_cfg.DataSize = 9;
	AD9910_ReadWrite_REG(&SPROFILE6_cfg, spiRxBuf, SPROFILE6_cfg.DataSize);
}
void AD9910_SingleTone_PF7(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite)
{
	REG_TypeDef SPROFILE7_cfg;
	//uint16_t amp_scale = 0xFFFF;
	
	SPROFILE7_cfg.RegAddr = (bReadWrite == 1) ? (PROFILE7_REG|CMD_READ):(PROFILE7_REG|CMD_WRITE);
	SPROFILE7_cfg.Data[7] = (uint8_t)(ftword >> 0);
	SPROFILE7_cfg.Data[6] = (uint8_t)(ftword >> 8);
	SPROFILE7_cfg.Data[5] = (uint8_t)(ftword >> 16);
	SPROFILE7_cfg.Data[4] = (uint8_t)(ftword >> 24);
	SPROFILE7_cfg.Data[3] = (uint8_t)(phase_offset >> 0);
	SPROFILE7_cfg.Data[2] = (uint8_t)(phase_offset >> 8);
	SPROFILE7_cfg.Data[1] = (uint8_t)(amp_scale >> 0);
	SPROFILE7_cfg.Data[0] = (uint8_t)(amp_scale >> 8);
	SPROFILE7_cfg.DataSize = 9;
	AD9910_ReadWrite_REG(&SPROFILE7_cfg, spiRxBuf, SPROFILE7_cfg.DataSize);
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_DIGR_LIMIT
//	Description : digital ramp generator upper/lower limit
//	Arguments :
//		up_limit:
//		low_limit:
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_DIGR_LIMIT(uint32_t up_limit, uint32_t low_limit, uint8_t bReadWrite)
{
	REG_TypeDef digr_limit;
	digr_limit.RegAddr = (bReadWrite == 1) ? (DIGR_LIMIT_REG|CMD_READ):(DIGR_LIMIT_REG|CMD_WRITE);
	digr_limit.Data[7] = (uint8_t)(low_limit >> 0);		//lower limit
	digr_limit.Data[6] = (uint8_t)(low_limit >> 8);		//lower limit
	digr_limit.Data[5] = (uint8_t)(low_limit >> 16);	//lower limit
	digr_limit.Data[4] = (uint8_t)(low_limit >> 24);	//lower limit
	digr_limit.Data[3] = (uint8_t)(up_limit >> 0);		//upper limit
	digr_limit.Data[2] = (uint8_t)(up_limit >> 8);		//upper limit
	digr_limit.Data[1] = (uint8_t)(up_limit >> 16);		//upper limit
	digr_limit.Data[0] = (uint8_t)(up_limit >> 24);		//upper limit
	digr_limit.DataSize = 9;
	AD9910_ReadWrite_REG(&digr_limit, spiRxBuf, digr_limit.DataSize);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_DIGR_STEPSIZE
//	Description : digital ramp generator positive/negative step size
//	Arguments :
//		pos_step:
//		neg_step:
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_DIGR_STEPSIZE(uint32_t pos_step, uint32_t neg_step, uint8_t bReadWrite)
{
	REG_TypeDef digr_stepsize;
	digr_stepsize.RegAddr = (bReadWrite == 1) ? (DIGR_STEPSIZE_REG|CMD_READ):(DIGR_STEPSIZE_REG|CMD_WRITE);
	digr_stepsize.Data[7] = (uint8_t)(pos_step >> 0);	//increment step size
	digr_stepsize.Data[6] = (uint8_t)(pos_step >> 8);	//increment step size
	digr_stepsize.Data[5] = (uint8_t)(pos_step >> 16);	//increment step size
	digr_stepsize.Data[4] = (uint8_t)(pos_step >> 24);	//increment step size
	digr_stepsize.Data[3] = (uint8_t)(neg_step >> 0);	//decrement step size
	digr_stepsize.Data[2] = (uint8_t)(neg_step >> 8);	//decrement step size
	digr_stepsize.Data[1] = (uint8_t)(neg_step >> 16);	//decrement step size
	digr_stepsize.Data[0] = (uint8_t)(neg_step >> 24);	//decrement step size
	digr_stepsize.DataSize = 9;
	AD9910_ReadWrite_REG(&digr_stepsize, spiRxBuf, digr_stepsize.DataSize);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_DIGR_RATE
//	Description : digital ramp generator positive/negative ramp rate
//	Arguments :
//		pos_rate:
//		neg_rate:
//		bReadWrite: read=1,write=0
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_DIGR_RATE(uint16_t pos_rate, uint16_t neg_rate, uint8_t bReadWrite)
{
	REG_TypeDef digr_rate;
	digr_rate.RegAddr = (bReadWrite == 1) ? (DIGR_RATE_REG|CMD_READ):(DIGR_RATE_REG|CMD_WRITE);
	digr_rate.Data[3] = (uint8_t)(pos_rate >> 0);		//positive ramp rate
	digr_rate.Data[2] = (uint8_t)(pos_rate >> 8);		//positive ramp rate
	digr_rate.Data[1] = (uint8_t)(neg_rate >> 0);		//neagtive ramp rate
	digr_rate.Data[0] = (uint8_t)(neg_rate >> 8);		//neagtive ramp rate
	digr_rate.DataSize = 5;
	AD9910_ReadWrite_REG(&digr_rate, spiRxBuf, digr_rate.DataSize);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	Function: AD9910_ReadWrite_REG
//	Description : write AD9910 register by SPI interface
//	Arguments :
//		reg : register definition
//		length : data length(bytes) to be sent
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void AD9910_ReadWrite_REG(REG_TypeDef *reg, uint8_t *spiRxData ,uint8_t length)
{
	uint8_t i;
	volatile uint8_t block[64];
	uint8_t *pReg = (uint8_t*)reg;

//	printf("Write_REG: len=%d ",(unsigned int)length);
	for (i=0; i<length; i++) {
//		printf("0x%02x ",pReg[i]);
		block[i] = pReg[i];
		}
//	printf("\n\r");

	CS_DDS_set_level(high);						//abort previous communication
	 delay_ms(1);		//delay 1ms
//	AD9910_IO_RESET_L;		//the next byte is instruction byte

	 delay_ms(1);		//delay 1ms
	CS_DDS_set_level(low);			//select AD9910, SPI transmission start
	 delay_ms(1);		//delay 1ms
	
	
	//HAL_SPI_Transmit(AD9910_SPI_HandlerPtr, (uint8_t *)reg, length, AD9910_SPI_TIMEOUT);
	//HAL_SPI_TransmitReceive(AD9910_SPI_HandlerPtr, (uint8_t *)reg, spiRxData, length, AD9910_SPI_TIMEOUT);

 SPI_0_exchange_block((void *)&block[0], length);

	for (i=0; i<length; i++) {
		spiRxData[i] = block[i];
		printf("Read_REG: len=%d ",(unsigned int)length);
		printf("0x%02x ",block[i]);
		printf("\n\r");
		}

#if 0
	for(i = 0; i < length; i++ )
	{
		spiRxData[i] = hmi_spi1_rwbyte(pReg[i]);
	}
#endif
	 delay_ms(1);		//delay 1ms
	CS_DDS_set_level(high);					//de-select AD9910, SPI transmission end
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Function : AD9910_IO_UPDATE
//Description : update buffer to I/O register
//Arguments: N/a
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// not implemented in freqref
void AD9910_IO_UPDATE(void)
{
#if 0
	//update IO buffer to register
	AD9910_IO_UPDATE_L;					//set IO_UPDATE low
	 delay_ms(1);					//delay 1ms
	AD9910_IO_UPDATE_H;					//set IO_UPDATE high
	 delay_ms(1);					//delay 1ms
	AD9910_IO_UPDATE_L;					//set IO_UPDATE low
	 delay_ms(1);					//delay 1ms
#else
//	printf("AD9910_IO_UPDATE(void) MISSING\n\r");
#endif
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Function : AD9910_SET_PROFILE
//Description : select profiles
//Arguments: profile number(0x00~0x07)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// not implemented in freqref hardware

void AD9910_SET_PROFILE(uint8_t profile)
{
#if 0
	switch(profile)
	{
		case(0):
		{
			AD9910_PROFILE0_L;
			AD9910_PROFILE1_L;
			AD9910_PROFILE2_L;
			break;
		}
		case(1):
		{
			AD9910_PROFILE0_H;
			AD9910_PROFILE1_L;
			AD9910_PROFILE2_L;
			break;
		}
		case(2):
		{
			AD9910_PROFILE0_L;
			AD9910_PROFILE1_H;
			AD9910_PROFILE2_L;
			break;
		}
		case(3):
		{
			AD9910_PROFILE0_H;
			AD9910_PROFILE1_H;
			AD9910_PROFILE2_L;
			break;
		}
		case(4):
		{
			AD9910_PROFILE0_L;
			AD9910_PROFILE1_L;
			AD9910_PROFILE2_H;
			break;
		}
		case(5):
		{
			AD9910_PROFILE0_H;
			AD9910_PROFILE1_L;
			AD9910_PROFILE2_H;
			break;
		}
		case(6):
		{
			AD9910_PROFILE0_L;
			AD9910_PROFILE1_H;
			AD9910_PROFILE2_H;
			break;
		}
		case(7):
		{
			AD9910_PROFILE0_H;
			AD9910_PROFILE1_H;
			AD9910_PROFILE2_H;
			break;
		}
	}
#endif
}
