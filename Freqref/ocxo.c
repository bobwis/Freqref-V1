/*
* ocxo.c
*
* Created: 26/09/2017 7:54:45 AM
*  Author: bob
*/

// Uses custom hardware connected to Arduino ATMEGA
// SPI Based 12 bit dual DAC MCP4822 on SPI bus to set the control voltage for the OCXO adjustment
// Two 32 bit digital counters connected via 8 bits  IO bus to Port C
// Used as count comparators between the GPS 10MHz clock and the OCXO 10MHz clock

#include <spi_basic.h>
#include <stdio.h>
#include "timeutils.h"
#include "ocxo.h"

#define __delay_cycles __builtin_avr_delay_cycles

// MCP4822 Dual DAC, ChA is dominant, ChB is fine voltage
// Set so ChA is full range, 2047 gives bout 2.012 Volts on the TCXO, ie 100 = 0.1V step
// ChB full 0-4095 range is about + or - 0.002V against the ChA output (not used)

unsigned long ocxocount, gpscount, ocxointerval;
unsigned int adcval = 2570;
bool ocxounlock = true;

// swap the endian of a byte
static  unsigned char niblookup[16] = {
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

unsigned char swapend(unsigned char n)
{

	// Reverse the top and bottom nibble then swap them.
	return (niblookup[n & 0xf] << 4) | niblookup[n >> 4];
}

void spiwrite16(uint16_t data)
{
	uint8_t  block[2];

	CS_DAC_set_level(low);
	delay_ms(2);
	block[0] = data >> 8;
	block[1] = data & 0xff;
	SPI_0_write_block(block, 2);
	while (SPI_0_status_busy())
	{
		;
	}
	CS_DAC_set_level(high);
}

#undef DEBUGDAC
#ifdef DEBUGDAC
void debugdac()
{
	int i;
	int cha, chb;
	do
	{
		cha = 0x1000 | 2000;	// 50 %
		spiwrite16(cha);    // chan A

		i = 0;
		for (;;)
		{
			LED_toggle_level();

			if (i == 0)
			i = 4095;
			else
			i = 0;
			printf("DAC %d\n\r",i);
			spiwrite16(0x8000 | 0x1000 | i);    // analog into tcxo
			delay_ms(5000/4);
		}
	} while (1);
}
#endif

// set up the ocxo DAC and IO ready for operation
bool ocxoinit()
{

	printf("starting DAC\n\r");
	SPI_0_enable();
	CS_DAC_set_level(low);
	if (!SPI_0_status_free())
	{
		return(false);
	}
	//	debugdac();
	adcval = 2580;
	spiwrite16(0x1000 | adcval);    // mid-ish voltage into tcxo control
	ocxointerval = 10000L;
	CNT_CLR_set_level(high);
	ocxounlock = true;
	return(true);
}

/*
Read the 4 bytes = 32 bits out of each counter
assumes count is already latched
*/
unsigned long read32cnt(int countnum)
{
	int i;
	unsigned char sel, n;
	unsigned long result;
	char selmap[] = {0, 4, 2, 6, 1, 5, 3, 7};

	result = 0;
	sel = 0;

	for (i = 3; i >= 0; i--)
	{
		sel = (countnum) ? i + 4 : i;
		sel = selmap[sel];
		PORTA = (sel << 2) | (PORTA & 0xC0);    // set the 3 to 8 encoder output
		__delay_cycles(100);

		result <<= 8;
		n = PINC;		// read 8 bits, but all the bits are wrong endian!
		//	n = swapend(n);
		result = result | n;
	}
	return (result);
}



// capture the current count into the registers and then clear the counters
void resetcnt()
// 0x40 is /CCLR
// 0x80 is RCLK
{
	CNT_CLR_set_level(high);		// ensure clr is inactive
	__delay_cycles(1);
	CNT_TFR_set_level(low);			// arm capture
	__delay_cycles(1);
	CNT_TFR_set_level(high);		// capture counters
	__delay_cycles(1);
	//	CNT_CLR_set_level(low);		// reset the counters
}


// test the counters
void testcounters()
{
	unsigned int adcval;
	int lasterr, err;

	adcval = 2580;		// a good starting point
	lasterr = err = 0;
	ocxointerval = 10000L;
	resetcnt();
	for(;;)
	{
		resetcnt();
		CNT_CLR_set_level(low);
		CNT_CLR_set_level(high);
		delay_ms((unsigned int)ocxointerval);

		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);
		err = gpscount-ocxocount;
		printf("ocxo=%08lu, gps=%08lu diff=%d\n\r",ocxocount,gpscount,err);
		ladder();

		//	if (lasterr != err)
		{
			adcval = adcval+err;
			spiwrite16(0x1000 | adcval);    // adjust voltage into tcxo control
			printf("dac=%d\n\r",adcval);
			lasterr = err;

		}
	}
}

// proportional ocxo
// the most basic algorithm
// this gets called periodically
void propocxo()
{
	int lasterr, err;
	static uint64_t lasttime = 0L;

	if ((lasttime + ocxointerval) < msectime())
	{
		lasttime = msectime();
		resetcnt();
		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);

		err = gpscount-ocxocount;

		if ((abs(err)) > 3)
		{
			ocxointerval = (ocxointerval > 1000) ? ocxointerval >> 1 : 1000;
		}
		else
		if ((abs(err)) < 2)
		{
			ocxointerval = (ocxointerval < 100000) ? ocxointerval << 1 : 100000;
		}

		if ((abs(err)) <= 1)
		{
			ocxounlock = false;
		}
		else
		{
			ocxounlock = true;
		}

		printf("ocxo=%08lu, gps=%08lu diff=%d interval=%ld\n\r",ocxocount,gpscount,err,ocxointerval);
		{
			adcval = adcval+err;
			spiwrite16(0x1000 | adcval);    // adjust voltage into tcxo control
			printf("dac=%d\n\r",adcval);
			lasterr = err;
		}
	}
}