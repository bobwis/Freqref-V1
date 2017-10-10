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
#include "pid.h"
#include "genpid.h"

#define __delay_cycles __builtin_avr_delay_cycles

// MCP4822 Dual DAC, ChA is dominant, ChB is fine voltage
// Set so ChA is full range, 2047 gives bout 2.012 Volts on the TCXO, ie 100 = 0.1V step
// ChB full 0-4095 range is about + or - 0.002V against the ChA output (not used)

unsigned long ocxocount, gpscount, ocxointerval;
unsigned int dacval = REFVAL;
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

// set the dac to the ocxo and wait a bit for the ocxo to generate the new frequency before returning
void setdacandwait(int adac, int bdac)
{
	int cha, chb;
	uint_fast64_t now;

	adac &= 0x0fff;
	bdac &= 0xffff;
	cha = 0x1000 | adac;	// 50 %
	spiwrite16(cha);    // chan A
	//	printf("setdacandwait DAC %d\n\r",i);
	spiwrite16(0x9000 | bdac);    // ch B
	delay_ms(250);		// dac=>ocxo output settle
}

//#undef DEBUGDAC
//#ifdef DEBUGDAC
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



//#endif


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
	dacval = REFVAL;
	spiwrite16(0x1000 | dacval);    // mid-ish voltage into tcxo control
	ocxointerval = 1024L;		// milli-seconds
	resetcnt();
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

// Capture the counters into the registers
void capturecnt()
{
	CNT_TFR_set_level(low);			// arm capture
	__delay_cycles(5);
	CNT_TFR_set_level(high);		// capture counters
	__delay_cycles(5);
}

// clear the counters
void resetcnt()
// 0x40 is /CCLR
// 0x80 is RCLK
{
	CNT_CLR_set_level(high);		// ensure clr is inactive
	__delay_cycles(5);
	CNT_CLR_set_level(low);		// reset the counters
	__delay_cycles(5);
	CNT_CLR_set_level(high);		// re-arm the clear
}

// 'tracking' front panel indicator
// this may be enhanced with hysteresis in future
void reportrack(long err)
{
	ocxounlock = (abs(err) <= 1) ? false : true;			// front panel warning lamp
}

// check the counters are not reading an offset (usually of 1) due to their asynchronous clocks
long deglitcherr(long err)
{
	long lasterr = err, prelasterr = err;
	uint8_t i;

	for (i=0; i<15; i++)			// asynchronous clock edges detection (ie poss timing count error of 1)
	{
		//		glitch = i;				// diagnostic
		capturecnt();
		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);
		err = gpscount-ocxocount;		// pos means ocxo is slower, so dac needs increase
		if ((err == lasterr) && (err == prelasterr))	// all three agree
		{
			if (i > 1)
				printf(" glitch %d ",i);
			return(err);						// last two readings have same error - therefore probable no clock timing glitch?
		}
		prelasterr = lasterr;
		lasterr = err;
	}
	return(err);		// readings always changing - error
}

// sample using averaging over 16 samples
long sample(void)
{
	long gpsup = 0, ocxoup = 0, zeroup = 0, err = 0;
	uint8_t i;

	for (i=0; i<32; i++)			// asynchronous clock edges detection (ie poss timing count error of 1)
	{
		capturecnt();
		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);

		if (gpscount > ocxocount)
			gpsup += gpscount - ocxocount;		// add the error
		else
		if (gpscount < ocxocount)
			ocxoup += ocxocount - gpscount;
		else // its zero
		{
			zeroup++;		// this won't do much until the real error is small
		}
		delay_ms(100);
	}

	printf("sample: gpsup=%ld, ocxoup=%ld, zeroup=%ld,",gpsup,ocxoup,zeroup);
	gpsup >>= 5;
	ocxoup >>= 5;
	zeroup >>= 5;

	err = (gpsup-ocxoup);
	if (zeroup > abs(err))
		err = 0;

	printf("returning %ld,\n\r",err);	
	return(err);
}

#if 0
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
#endif


#if 1
// proportional ocxo control
// the most basic algorithm
// this gets called periodically
void propocxo()
{
	int i, err, lasterr, glitch;
	unsigned int magerr;
	static uint64_t lasttime = 0L, lastcheck = 0;
	int scale;
	
	if ((ocxointerval > 16384L) && (lastcheck + 16384L) < msectime())		// we are going to do a quick check anyway
	{
		lastcheck = msectime();
		capturecnt();
		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);
		err = gpscount-ocxocount;		// pos means ocxo is slower, so dac needs increase
		if (abs(err) >= 3)				// something made a big unexpected reading change
		{
			ocxointerval = 16384L;		// put the correct interval in
			lasttime = 0L;				// force processing early
		}
	}

	if ((lasttime + ocxointerval) < msectime())
	{
		lasterr = 55555;
		for (i=0; i<5; i++)			// asynchronous clock edges detection (ie poss timing count error of 1)
		{
			glitch = i;
			lasttime = msectime();
			lastcheck = lasttime;
			capturecnt();
			ocxocount = read32cnt(0);
			gpscount = read32cnt(1);
			err = gpscount-ocxocount;		// pos means ocxo is slower, so dac needs increase
			if (err == lasterr)
			break;						// last two readings have same error - therefore probablt no clock timing glitch?
			lasterr = err;
		}

		resetcnt();		// zero the counters
		magerr = abs(err);

		if (magerr > 3)
		{
			ocxointerval = (ocxointerval > 4096) ? ocxointerval >> 1 : 2048;	// reduce time by half
		}
		else
		if (magerr <= 2)
		{
			ocxointerval = (ocxointerval <= 256000L) ? (ocxointerval << 1) :  420000;		// add 100% more time
		}

		reportrack(err);		// front panel lamp

		scale = (540L - ((ocxointerval*4L) / 1000L)) / 8;		// scale dac correction for sample time
		if (scale < 1)
		{
			scale = 1;
		}
		magerr =  magerr * scale;		// scale

		if (magerr > 4095)
		{
			magerr = 4095;		// limit dac step size
		}

		if (err < 0)
		{
			dacval = dacval - magerr;
		}
		else if (err > 0)
		{
			dacval = dacval + magerr;
		}

		if (dacval > 0xfff)
		dacval = 0xfff;
		
		setdacandwait(dacval,2048);    // adjust voltage into tcxo control
		//		printf("P ocxo=%08lu, gps=%08lu diff=%d, magerr=%d interval=%ld DAC=%u\n\r",ocxocount,gpscount,err,magerr,ocxointerval,dacval);

		printf("\"uptime\",%lu,\"ocxo\",%08lu,\"gps\",%08lu,\"err\",%d,\"scale\",%d,\"magerr\",%d,\"interval\",%lu,\"DAC\",%d,\"glitch\",%d\n\r",
		(unsigned long)msectime(),ocxocount,gpscount,err,scale,magerr,ocxointerval,(unsigned)dacval,glitch-1);
	}
}
#endif


// paul's pid
//to use it call compute()  pass in the interval which you took the count over and the 2 counts
void pid(void)
{
	static uint64_t lasttime = 0L, lastcheck = 0L, interval = 2000L;
	int scale;
	unsigned int dac;
	static float TOLERANCE = 1e-5;
	float dacf;
	
	if ((lastcheck + interval) < msectime())		// we are going to do a check
	{
		lastcheck = msectime();

		capturecnt();
		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);

		dacf = compute((uint64_t) interval, (float)gpscount, (float)ocxocount);
		dac = (uint16_t) dacf;
		setdacandwait(dac,2048);    // adjust voltage into tcxo control

		if (abs(lasterr) < TOLERANCE)
		{
			interval += interval*2 ;
		}
		else
		{
			interval = 2000;
		}
		resetcnt();
		printf("PID2 DAC=%i, interval=%ld, ocxo=%08lu, gps=%08lu lasterr=%f\n\r",dac,(unsigned long)interval,ocxocount,gpscount,lasterr);
	}
}

// adjust the dac (scale and limit as appropriate from the error)
// err contains gpscnt - ocxocnt
uint16_t calcdac(long err)
{
	bool errneg;
	uint16_t dac;

	errneg = (err < 0L) ? true : false;
	err = (errneg) ? -err : err;			// make err positive magnitude
	if (err > 2047L)
	{
		err = 2047L;	// max value limit
	}

	dac = (errneg) ? dacval - err : dacval + err;
	if (dac > 4095)
	dac = 4095;
	return(dac);
}


// tracking ocxo control
// this gets called periodically
void track3ocxo()
{
	int scale;
	static int olddac = 0;
	uint16_t magerr;
	static long int err;
	static uint64_t lasthit = 0L, lastcap = 0, lasttime = 0;
	static uint8_t state = 0;
	// state 0 == start, 1 == ocxo settling, 2== waiting to sample, 3==sample, 4=error detected

	switch (state) {
		case 0:	// start of a new cycle
		lasthit = msectime();
		state = 2;
		break;

		case 1:		// ocxo settling
		delay_ms(200);
		resetcnt();
		state = 0;
		break;

		case 2:		// waiting to sample
		if ((lasthit + 10000L) < msectime())
		{
			lasthit = msectime();
			state = 3;
		}
		break;

		case 3:		// sample
		capturecnt();
		ocxointerval = msectime() - lastcap;		// sample interval time
		lastcap = msectime();
		err = sample();

		reportrack(err);		// front panel lamp

		if (abs(err) >= 1)  	// tracking possibly slipping
		{
			state = 4;
		}
		else
		{
			state = 0;
		}
		break;

		case 4:		// error detected

		magerr =  abs(err);
		
		scale = abs(2574 - dacval) / magerr;		// cheat
		if (scale < 1)
		{
			scale = 1;
		}

		magerr *=  scale;		// scale

		if (err < 0)
		{
			dacval = dacval - magerr;
		}
		else if (err > 0)
		{
			dacval = dacval + magerr;
		}

		if (dacval > 0xfff)
			dacval = 0xfff;

		setdacandwait(dacval,2048);    // adjust voltage into tcxo control

		printf("T3 DAC=%i, uptime=%3lu Sec, elapsed=%3lu Sec, interval=%3lu Sec, ocxo=%08lu, gps=%08lu err=%ld scale=%d\n\r",
			dacval,(unsigned long)msectime()/1000L,(unsigned long)(msectime()-(unsigned long)lasttime)/1000L,
			ocxointerval/1000L,ocxocount,gpscount,err,scale);
		lasttime = msectime();
		state = 1;

	}	// end case
}



// test functions -----------------------------------------------------------------------------------
long int getreading(unsigned int adac, unsigned int bdac, unsigned long timeout)
{
	volatile int64_t now, targettime;

	setdacandwait(adac,bdac);
	//		delay_ms(15000L);		// wait 15 seconds for settle

	resetcnt();		// zero the counters and start count again
	now = msectime();
	targettime = now + timeout;

	while(msectime() < targettime)
	;

	capturecnt();
	ocxocount = read32cnt(0);
	gpscount = read32cnt(1);
	return(gpscount - ocxocount);
}


// get some coefficients
void testocxo()
{
	long int err;

	//	err = getreading(0,400000L);
	err = getreading(2574,0,5000L);
	printf("Test DAC=%i, ocxo=%08lu, gps=%08lu diff=%ld \n\r",0,ocxocount,gpscount,err);

	//		err = getreading(0xfff,400000L);
	err = getreading(2574,4095,5000L);
	printf("Test DAC=%i, ocxo=%08lu, gps=%08lu diff=%ld \n\r",0xfff,ocxocount,gpscount,err);
}