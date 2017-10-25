// originally from Wade Chien
// https://ez.analog.com/message/322762-re-ad9910-drg-mode-in-amplitude-sweep-triangle-waveform-output-full-rang-different-from-single-tone?commentID=322762&et=watches.email.thread#comment-322762
// Adapted with permission

/*
 * ad9910.h
 *
 * Created: 24/10/2017 1:15:29 PM
 *  Author: bob
 */ 


#ifndef AD9910_H_
#define AD9910_H_

#define AD9910_SPI_TIMEOUT			10	//time out count =10
//*************************************************************************
//parameter for AD9910 register buffer 
#define SPI1_RX_BUF_SIZ		9
#define REG_DATA_BYTES		8
#define DAC_MAX_RESOLUTION 	0x3FFF
#define DISABLE_DRG		 	0x00		//no DRG function
#define ENABLE_DRG_TRI		0x01		//enable DRG with triangle wave function
#define ENABLE_DRG_SQU		0x02		//enable DRG with square wave function
#define ENABLE_DRG_SAW_UP	0x03		//enable DRG with sawtooth wave function(ramp up)
#define ENABLE_DRG_SAW_DOWN	0x04		//enable DRG with sawtooth wave function(ramp down)
#define SYSCLK				1000000000	//1GHz	
#define DDSCLK				SYSCLK/4	//250MHz	

//*******************AD9910 Register ADDRESS Map***************************
//------@Register Name--------------@Address	@Length of data
#define	bWRITE						0			
#define	bREAD						1	
#define	CMD_WRITE					0x00		//bit[7]=0:write instruction
#define	CMD_READ					0x80		//bit[7]=1:read instruction
#define	CRYSTAL_CLK					25000000	//AD9910 crystal clock = 25MHz
#define	AD9910_SYSCLK				1000000000	//AD9910 system clock = 1GHz
#define	PLL_MULTIPLY				(AD9910_SYSCLK/CRYSTAL_CLK)		
												//PLL multiply factor = system
#define	CFR1_REG					0x00		//4 bytes
#define	CFR2_REG					0x01		//4 bytes
#define	CFR3_REG					0x02		//4 bytes
#define	AUX_DAC_CTRL_REG			0x03		//4 bytes
#define	IO_UPDATE_RATE_REG			0x04		//4 bytes
#define	FTW_REG						0x07		//4 bytes
#define	POW_REG						0x08		//2 bytes
#define	ASF_REG						0x09		//4 bytes
#define	MLTI_CHIP_SYNC_REG			0x0A		//4 bytes
#define	DIGR_LIMIT_REG				0x0B		//8 bytes
#define	DIGR_STEPSIZE_REG			0x0C		//8 bytes
#define	DIGR_RATE_REG				0x0D		//4 bytes
#define	PROFILE0_REG				0x0E		//8 bytes
#define	PROFILE1_REG				0x0F		//8 bytes
#define	PROFILE2_REG				0x10		//8 bytes
#define	PROFILE3_REG				0x11		//8 bytes
#define	PROFILE4_REG				0x12		//8 bytes
#define	PROFILE5_REG				0x13		//8 bytes
#define	PROFILE6_REG				0x14		//8 bytes
#define	PROFILE7_REG				0x15		//8 bytes
#define	RAM_REG						0x16		//4 bytes
//*************************************************************************

//**********************************************************************//
//						AD9910 Register configuration					//
//**********************************************************************//
//*******************Control Function Register 1 (CFR1) **************************************************
//
//--CFR1_REG[7:0],DATA[0]--
#define LSB_FST			1				//(1 << 0)	LSB first
#define MSB_FST			0				//(0 << 0)	MSB first
#define SDIO_IPT		1				//(1 << 1)	SDIO input only (3-wire)
#define EXT_PWD			1				//(1 << 3)	fast recovery power down (0=full power down, default)
#define ADAC_PWD		1				//(1 << 4)	auxiliary DAC clock signal disabled (default active)
#define REFCLK_PWD		1				//(1 << 5)	REFCLK and PLL disabled (default active)
#define DAC_PWD			1				//(1 << 6)	DAC clock disabled (default active)
#define DIG_PWD			1				//(1 << 7)	clock to digital core disabled (default active) 
//--CFR1_REG[15:8],DATA[1]--
#define AUTO_OSK		1				//(1 << 0)	enable automatic OSK
#define OSKEN			1				//(1 << 1)	enable OSK
#define LOAD_ARR		1				//(1 << 2)	load ARR @ IO update
#define CLR_PHA			1				//(1 << 3)	clear phase accumulator
#define CLR_DIG			1				//(1 << 4)	clear digital ramp accumulator
#define ACLR_PHA		1				//(1 << 5)	auto clear phase accumulator
#define ACLR_DIG		1				//(1 << 6)	auto clear digital ramp accumulator
#define LOAD_LRR		1				//(1 << 7)	load LRR @ IO update
//--CFR1_REG[23:16],DATA[2]--
#define DDS_SINE		1				//(1 << 0)	DDS output = sinewave (default = cosine, 0)
#define DDS_COSINE		0				//(0 << 0)	DDS output = cosine (default = cosine, 0)
#define INVSIC			1				//(1 << 6)	enable inverse sinc filter
#define MANUAL_OSK		1				//(1 << 7)	enable manual OSK
//--CFR1_REG[31:24],DATA[3]--
#define RAM_ENABLE		1				//(1 << 7)	enable RAM function
//
//********************************************************************************************************

//*******************Control Function Register 2 (CFR2) **************************************************
//
//--CFR2_REG[7:0],DATA[0]--
#define PARL_ENA		1				//(1 << 4)	parallel data port enable
#define SYNC_DIS		1				//(1 << 5)	SYNC_SMP_ERR pin is forced to low
#define DATA_ASS		1				//(1 << 6)	data assembler of parallel data port to hold last value
#define MATCH_ENA		1				//(1 << 7)	match latency enable(no latency)
//--CFR2_REG[15:8],DATA[1]--
#define TX_ENA			1				//(1 << 1)	TxEnable inversion
#define PDCLK_INV		1				//(1 << 2)	Invert PDCLK polarity
#define PDCLK_ENB		1				//(1 << 3)	internal PDCLK signal appears at PDCLK pin
#define IOUPD_DIV_1		0				//(0 << 6)	IO update rate divide by 1
#define IOUPD_DIV_2		1				//(1 << 6)	IO update rate divide by 2
#define IOUPD_DIV_4		2				//(2 << 6)	IO update rate divide by 4
#define IOUPD_DIV_8		3				//(3 << 6)	IO update rate divide by 8
//--CFR2_REG[23:16],DATA[2]-
#define READ_FTW		1				//(1 << 0)	read FTW register 
#define DIGR_NDWELL_L	1				//(1 << 1)	enable no-dwell low function
#define DIGR_NDWELL_H	1				//(1 << 2)	enable no-dwell high function
#define DIGR_ENABLE		1				//(1 << 3)	enable digital ramp generator
#define DIGR_DES_BIT0	1				//(1 << 4)	DRG desitination bit0
#define DIGR_DES_BIT1	1				//(1 << 5)	DRG desitination bit1
#define SYNCCLK_ENA		1				//(1 << 6)	generate SYNC_CLK at 1/4 system clock
#define ITER_IOUPD		1				//(1 << 7)	Internal I/O update active
//--CFR2_REG[31:24],DATA[3]--
#define ENA_AMP			1				//(1 << 0)	enable amplitude scale from single tone profile
//
//********************************************************************************************************

//*******************Control Function Register 3 (CFR3) **************************************************
//
//--CFR3_REG[7:0],DATA[0]--
//[7:1]N=12~127=divide modulus defined in register configuration
//--CFR3_REG[15:8],DATA[1]--
#define PLL_ENA			1				//(1 << 0)	REFCLK PLL enable
#define PFD_RST			1				//(1 << 2)	phase detector disabled (must be reset normal operation)
#define REFCLK_DIV_BYP	1				//(1 << 3)	REFCLK input divider is bypassed (must be enabled if REFCLK>1GHz)
#define REFCLK_DIV_RST	0				//(1 << 4)	REFCLK input divider is reset (must be enabled if REFCLK>1GHz)
//--CFR3_REG[23:16],DATA[2]--
#define ICP_212UA		0				//(0 << 3)	charge pump current = 212uA
#define ICP_237UA		1				//(1 << 3)	charge pump current = 237uA
#define ICP_262UA		2				//(2 << 3)	charge pump current = 262uA
#define ICP_287UA		3				//(3 << 3)	charge pump current = 287uA
#define ICP_312UA		4				//(4 << 3)	charge pump current = 312uA
#define ICP_337UA		5				//(5 << 3)	charge pump current = 337uA
#define ICP_363UA		6				//(6 << 3)	charge pump current = 363uA
#define ICP_387UA		7				//(7 << 3)	charge pump current = 387uA
//--CFR3_REG[31:24],DATA[3]--
#define VCO0			0				//(0 << 0)	VCO0 370MHz to 510MHz
#define VCO1			1				//(1 << 0)	VCO1 420MHz to 590MHz
#define VCO2			2				//(2 << 0)	VCO2 500MHz to 700MHz
#define VCO3			3				//(3 << 0)	VCO0 600MHz to 880MHz
#define VCO4			4				//(4 << 0)	VCO0 700MHz to 950MHz
#define VCO5			5				//(5 << 0)	VCO0 820MHz to 1150MHz
#define RCLKOUT_DIS		0				//(0 << 4)	REFCLK output buffer disable
#define RCLKOUT_LOW		1				//(1 << 4)	REFCLK output buffer low current (default)
#define RCLKOUT_MED		2				//(2 << 4)	REFCLK output buffer medium current
#define RCLKOUT_HIG		3				//(3 << 4)	REFCLK output buffer high current
//
//********************************************************************************************************


#define	SET_PROFILE0	0x00
#define	SET_PROFILE1	0x01
#define	SET_PROFILE2	0x02
#define	SET_PROFILE3	0x03
#define	SET_PROFILE4	0x04
#define	SET_PROFILE5	0x05
#define	SET_PROFILE6	0x06
#define	SET_PROFILE7	0x07


//**********REGISTER STRUCTURE DECLRATION **********************
typedef struct  
{
	uint8_t RegAddr;				//register address
	uint8_t Data[REG_DATA_BYTES];	//register data (4 bytes)
	uint8_t DataSize;				//how many bytes of data
}REG_TypeDef;
//**************************************************************


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Function Prototype 								  
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//***AD9910 hardware API***
void AD9910_reset(void);
void AD9910_init(void);		 
void AD9910_single_tone(uint16_t amplitude, uint16_t phase_offset, uint32_t frequency, uint8_t set_prof);
void AD9910_triangle_wave(uint32_t freq, uint8_t duty);
void AD9910_drg_acc_clr(void);
void AD9910_pha_acc_clr(void);

//***AD9910 register setting***
//void AD9910_CFR1_config(uint8_t bReadWrite);
void AD9910_CFR1_config(uint8_t sinecos, uint8_t drg_acc_clr, uint8_t pha_acc_clr, uint8_t bReadWrite);
void AD9910_CFR2_config(uint8_t drg_function, uint8_t bReadWrite);
void AD9910_CFR3_config(uint8_t pll_multipier, uint8_t bPLLENAB, uint8_t bReadWrite);
void AD9910_AuxDAC_config(uint8_t DACCode, uint8_t bReadWrite);
void AD9910_IOupdate_config(uint8_t bReadWrite);
void AD9910_FTW_config(uint32_t ftword, uint8_t bReadWrite);
void AD9910_POW_config(uint16_t phase, uint8_t bReadWrite);
void AD9910_ASF_config(uint16_t amp_scale, uint8_t bReadWrite);
void AD9910_DIGR_LIMIT(uint32_t up_limit, uint32_t low_limit, uint8_t bReadWrite);
void AD9910_DIGR_STEPSIZE(uint32_t pos_step, uint32_t neg_step, uint8_t bReadWrite);
void AD9910_DIGR_RATE(uint16_t pos_rate, uint16_t neg_rate, uint8_t bReadWrite);
void AD9910_SingleTone_PF0(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite);
void AD9910_SingleTone_PF1(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite);
void AD9910_SingleTone_PF2(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite);
void AD9910_SingleTone_PF3(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite);
void AD9910_SingleTone_PF4(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite);
void AD9910_SingleTone_PF5(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite);
void AD9910_SingleTone_PF6(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite);
void AD9910_SingleTone_PF7(uint16_t amp_scale, uint16_t phase_offset, uint32_t ftword, uint8_t bReadWrite);


//***AD9910 register write (serial interface)***
void AD9910_ReadWrite_REG(REG_TypeDef *reg, uint8_t *spiRxData ,uint8_t length);
void AD9910_IO_UPDATE(void);
void AD9910_SET_PROFILE(uint8_t profile);

extern uint16_t dds_singletone_amp;		//preset default value(Brian add)
extern uint32_t dds_singletone_freq;	//preset default value(Brian add)
extern uint16_t dds_digr_duty;			//preset default value(Brian add)
extern uint32_t dds_digr_freq;			//preset default value(Brian add)

#define low false
#define high true

#endif /* AD9910_H_ */