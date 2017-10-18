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

// MCP4822 Dual DAC, ChA is dominant, ChB is fine voltage 1000/th of Dac A step
// Set so ChA is full range, 2047 gives bout 2.012 Volts on the TCXO, ie 100 = 0.1V step
// ChB full 0-4095 range is about + or - 0.002V against the ChA output (not used)

unsigned long ocxocount, gpscount, ocxointerval;
unsigned int dacval = REFVAL;
uint8_t ocxounlock = 2;
unsigned long warmupmins = 0;
unsigned long hotstarttime = 0;

#define SCALESTEPS 10
// scale factor for sample times
// sample seconds, multiplier scale
static  unsigned int samscale [SCALESTEPS][2] = {
	{1, 70},
	{2, 70},
	{5, 65},
	{10, 55},		// eg elapsed in range 10 to 19sec gives value 40
	{19, 40},
	{33, 11},
	{59, 5},
	{154, 3},
	{240, 2},
	{32767, 1}
};

#if 0
// swap the endian of a byte
static  unsigned char niblookup[16] = {
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

unsigned char swapend(unsigned char n)
{

	// Reverse the top and bottom nibble then swap them.
	return (niblookup[n & 0xf] << 4) | niblookup[n >> 4];
}
#endif


// wait for next sample in chunks of 1/3 sec
// keep ladder alive
#define CHUNKTIME 333
void dlyandladder(int chunk)
{
	while (chunk)
	{
		delay_ms(CHUNKTIME/2);
		ladder();
		delay_ms(CHUNKTIME/2);
		ladder();
		chunk--;
	}
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

	adac &= 0x0fff;
	bdac &= 0xffff;
	cha = 0x1000 | adac;
	chb = 0x9000 | bdac;
	spiwrite16(cha);    // chan A
	//	printf("setdacandwait DAC %d\n\r",i);
	spiwrite16(chb);    // chan B
	dlyandladder(1);		// dac=>ocxo output settle
}

//#undef DEBUGDAC
//#ifdef DEBUGDAC
void debugdac()
{
	int i;
	int cha;
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
	setdacandwait(REFVAL,REFVAL);		// eventually refval will be from EEPROM
	ocxointerval = 1024L;		// milli-seconds
	resetcnt(1);
	ocxounlock = 2;
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
// scale is the A DAC step multiplier currently in use
void resetcnt(int currscale)
// 0x40 is /CCLR
// 0x80 is RCLK
{
	currscale = 3 - currscale;		// we want the low scales to have bigger delay
	if (currscale <= 0)			// to try to prevent premature counter noise being accepted
		currscale = 1;
	if (currscale < 3)
		currscale <<= 2;			// so scale of 1 will now give us 8. 2 will give 4

	for(;;)
	{
		CNT_CLR_set_level(high);		// ensure clr is inactive
		__delay_cycles(25);
		CNT_CLR_set_level(low);		// reset the counters
		__delay_cycles(25);
		CNT_CLR_set_level(high);		// re-arm the clear

		capturecnt();		// freeze the early count
		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);
		if (gpscount == ocxocount)		// if they dont start out the same have another go
		{
			break;
		}
		else
		{
			//			printf("ResetCNT retried DAC=%i, ocxo=%08lu, gps=%08lu\n\r",dacval,ocxocount,gpscount);
		}
		dlyandladder((currscale*6000)/CHUNKTIME);		// wait for gps counters to cover some time to help reduce sequence count noise
	}
}

// 'tracking' front panel indicator
// this may be enhanced with hysteresis in future
void reportrack(long err, int scale)
{
	ocxounlock = 2;		// front panel warning lamp
	if (scale == 2)
	{
		if (abs(err) <= 1)
		{
			ocxounlock = 1;	// we are nearly there 
		}
	}
	else
	if (scale == 1)
	{
		if (abs(err) <= 1)
		{
			ocxounlock = 0;	// we are tracking...
		}
	}
}

#if 0
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
#endif

void readcounters(void)
{
	int i;

	for(i=0; i<3; i++)
	{
		capturecnt();
		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);

		if (abs(gpscount - ocxocount) < 0x4000) 	// window: wrapped counter or one failed
			break;
	}
}

// sample using averaging over n samples
// need running avg to limit glitches
// this is used by the tracking v3 routine
#define SAMPLES	16
long sample(void)
{
	long zeroup = 0, newerr = 0, err = 0, maxr, minr,  warmtime;
	static long lasterr = 999;
	int i, j = 0, k = 0;

	i = 0;
	maxr =0;
	minr = 0;
	while (i < SAMPLES)			// asynchronous clock edges detection (ie poss timing count error of 1)
	{
		capturecnt();
		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);

		newerr = gpscount - ocxocount;
		
		while (abs(newerr) != abs(lasterr)) 		// wait for two readings the same, or exact opposite
		{
			readcounters();		// cntrs into oxxocount and gpscount
			lasterr = newerr;
			newerr = gpscount - ocxocount;
			j++;
		}

		if (newerr != 0)
		{
			if (newerr > maxr)
			maxr = newerr;
			else
			if (newerr < minr)
			minr = newerr;
			err = err + newerr;
			i++;
		}
		else // its zero
		{
			zeroup++;		// this won't do much until the real error is small
			lasterr = 0;
			i++;	/// needed?
		}

		if ((msectime() + hotstarttime) < ((WARMINGTIME)*60L*1000L))			// not gone past warmup period
		{
			warmtime = (uint32_t)(msectime()+hotstarttime);			// from start up in ms
		}
		else
		{
			warmtime = ((3*60000)<<3);		// 3 lots of 333ms
		}
		ocxointerval = (((warmtime/(1000L*60L))>>2)*CHUNKTIME);
		dlyandladder((warmtime/(1000L*60L))>>3);			// seconds delay
	}

	lasterr = 9999;
	while (abs(newerr) != abs(lasterr)) 		// wait for two readings the same, or exact opposite
	{
		capturecnt();
		ocxocount = read32cnt(0);		// cnts can be drifted
		gpscount = read32cnt(1);
		lasterr = newerr;
		newerr = gpscount - ocxocount;
		if (newerr == 0)
		zeroup++;
		err = err + newerr;
		k++;
		i++;
	}

	err = err + minr - maxr;		// remove the two extreme errors from the plot
	i -= 2;
	
	if ((abs(err) > (SAMPLES)) && (zeroup >= 1))	// error is too big - must be glitch
	{
		//		printf("LIMIT err=%ld ",err);
		err = 0;
	}

	//	printf("sample: err=%ld, zeroup=%ld, i=%d, maxr=%ld minr=%ld j=%d, k=%d ",err,zeroup,i,maxr,minr,j,k);

	err /= (i>>2);		//  average all samples DIV 2
	//	err /= i;		//  average all samples

	if (zeroup > (i - (i>>2)))		// more than 75% samples were zero
	{
		//		printf("Zero wins, ");
		hotstarttime += 60000L;		// + 1 min
		err = 0;
	}

	//	printf("returning %ld\n\r",err);
	return(err);
}




#if 0
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

		scale = (540L - ((ocxointerval*4L) / 1000L)) / 8;		// scale dac correction for sample time
		
		if (scale < 1)
		{
			scale = 1;
		}

		reportrack(err,scale);		// front panel lamp

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

#if 0
// paul's pid
//to use it call compute()  pass in the interval which you took the count over and the 2 counts
void pid(void)
{
	static uint64_t lastcheck = 0L, interval = 2000L;
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
#endif

#if 0
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
#endif

// tracking ocxo control
// this gets called periodically
void track3ocxo()
{
	static unsigned int scale = 5, tim, i, oldbdac = 0;
	static int lastscaleidx = 1, resetbackoff = 7;
	int bdac;
	uint16_t magerr;
	static long int err;
	static uint64_t lasthit = 0L, lastcap = 0, lasttime = 0;
	static uint8_t state = 0;
	// state 0 == start, 1 == ocxo settling, 2== waiting to sample, 3==sample, 4=error detected, 5=BDac adjustment

	switch (state) {
		case 0:	// start of a new cycle
		lasthit = msectime();
		state = 2;
		break;

	case 1:		// ocxo settling then reset counters
		dlyandladder(1);
//		if (scale != 1)			// this tweak *should* work when response is improved
			resetcnt(scale);	// currently it oscillates around the optimum point
		state = 0;
		break;

	case 2:		// waiting to sample
		if ((lasthit + 100) < msectime())
		{
			lasthit = msectime();
			state = 3;
		}
		break;

	case 3:		// sample
		ocxointerval = msectime() - lastcap;		// sample interval time
		lastcap = msectime();
		err = sample();

		if ((abs(err) < 4) && (scale <= 1))		// sub A Dac error, so use B Dac
		{
			state = 5;
			break;
		}

		err /= 4;		// A dac resolution
		
		if (abs(err) >= 1)  	// tracking possibly slipping
		{
			state = 4;			// process the error
		}
		else  		// err == 0
		{
			if (((unsigned long)(msectime()-(unsigned long)lasttime)/1000L) > 300000L) // long time no err
			{
				scale = 1;		// if the last big guess jumps right to the sweet spot Dac val
				lastscaleidx = SCALESTEPS-1;
			}
			state = 0;			// start a new sample cycle
		}
		reportrack(err,scale);		// front panel lamp

		break;

	case 4:		// error detected, set the A DAC and reset counters etc

		magerr =  abs(err);
		tim = (unsigned long)(msectime()-(unsigned long)lasttime)/1000L;

		i = 0;
		while (i < SCALESTEPS)	// error is scaled depending up last interval
		{
			if (samscale[i][0] > tim)			// find optimum solution a bit quicker than sample() does
			{
				scale = samscale[i][1];			// new scale found, shall we use this?
				break;
			}
			i++;
		}

		if ((lastscaleidx > (i)) && (lastscaleidx > (SCALESTEPS-3)) && (err <= 2))			// last sample time was longer than this
		{
			scale = samscale[lastscaleidx][1];	// keep old one
		}
		else
			lastscaleidx = i;		// save i as this one is higher than before

		magerr *=  scale;		// scale

		if (err < 0)		// dac will step down
		{
			dacval = dacval - magerr;
		}
		else if (err > 0)
		{
			dacval = dacval + magerr;
		}

		if (dacval > 0xfff)
		dacval = 0xfff;

		if (magerr == 1)		// just one step on the big Dac
		{
			if (err < 0)
			oldbdac = oldbdac + 500;	// try a smooth voltage transition
			else
			oldbdac = oldbdac - 500;	// by compensating 1/2 step on the B dac

			if ((oldbdac < 0) || (oldbdac > 4095))
			oldbdac = dacval;
		}
		else
		{
			oldbdac = dacval;
		}
		bdac = oldbdac;

		setdacandwait(dacval,bdac);    // adjust voltage into tcxo control

		reportrack(err,scale);		// front panel lamp - shouldn't be needed here?

		printf("T3 ADAC=%i, BDAC=%i, uptime=%3lu Sec, elapsed=%3lu Sec, interval=%3lums, ocxo=%08lu, gps=%08lu err=%ld scale=%d\n\r",
		dacval,bdac,(unsigned long)msectime()/1000L,(unsigned long)(msectime()-lasttime)/1000L,
		ocxointerval,ocxocount,gpscount,err,scale);
		lasttime = msectime();
		//		if ((unsigned long)((msectime()-lasttime)/1000L) > 250L)			// for safety only

		resetbackoff = 999;	// redundant
		state = 1;			// reset the counters etc
		break;

	case 5:			// B dac fine adjustment
		if (err != 0)		// err = -3 to + 3
		{
			bdac = dacval + err * 333;		// range -500 to +500 with -4 to +4 input
			bdac = (bdac + oldbdac) / 2;	// average with last times
			if (oldbdac != bdac)
			{
				printf("Fine: count=%d, ADAC=%d BDAC=%d scale=%u, err=%ld \n\r",resetbackoff,dacval,bdac,scale,err);
				spiwrite16(0x9000 | bdac);    // ch B
				oldbdac = bdac;
			}
			if ((resetbackoff == 0) || (bdac > 3595) || (bdac < 500))	// refresh
			{
				state = 1;			// reset the counters this time
				resetbackoff = 999;
			}
			else
			{
				resetbackoff--;
				state = 0;			// normal cycle next
			}
		}
		else
		state = 0;
		break;

	}	// end case
}

#if 0
/****************************************************************************************************/
// test functions -----------------------------------------------------------------------------------
void findglitch(unsigned int adac, unsigned int bdac, unsigned long timeout)
{
	static unsigned long lastgps, lastocxo;
	unsigned long gpsdiff, ocxodiff;
	unsigned long ocxostep, gpsstep, now2;

	setdacandwait(adac,bdac);
	//		delay_ms(15000L);		// wait 15 seconds for settle

	resetcnt();		// zero the counters and start count again

	#define INTERVAL 200L
	#define LIMIT 1

	ocxostep = INTERVAL * 10000L;		// estimate 10MHz times interval in mSec
	gpsstep = INTERVAL * 10000L;
	for (;;)
	{
		sei();
		delay_ms(INTERVAL);

		capturecnt();
		now2 = (unsigned long) (msectime() & 0xffffffff);

		ocxocount = read32cnt(0);
		gpscount = read32cnt(1);

		ocxodiff = ocxocount - lastocxo;
		gpsdiff = gpscount - lastgps;

		if ((ocxodiff > (gpsdiff + LIMIT)) || (ocxodiff < (gpsdiff - LIMIT)))	// diff between counters
		{
			//			printf("x=%ld, g=%ld d=%ld\r\n",ocxostep,ocxocount - lastocxo,ocxostep-(ocxocount - lastocxo));
			printf("now=%6lu, ocxocount=%lu, gpscount=%6lu, ocxodiff=%lu, gpsdiff=%lu err=%ld\r\n",(uint32_t)now2,ocxocount,gpscount,ocxodiff,gpsdiff,ocxodiff-gpsdiff);
		}
		
		ocxostep = (ocxostep>>1) + ((ocxocount - lastocxo)>>1);		// moving avg
		gpsstep = (gpscount>>1) + ((gpscount - lastgps)>>1);

		lastocxo = ocxocount;
		lastgps = gpscount;
	}
}



long int getreading(unsigned int adac, unsigned int bdac, unsigned long timeout)
{
	volatile int64_t now, targettime;

	setdacandwait(adac,bdac);

	resetcnt();		// zero the counters and start count again
	now = msectime();
	targettime = now + timeout;

	while(msectime() < targettime)
	ladder();

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

void testglitch()
{

	findglitch(2820, 2820,1000L);
}
#endif

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