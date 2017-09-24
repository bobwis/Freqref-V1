/*
* ladder.c
*
* Created: 22/09/2017 12:02:12 PM
*  Author: bob
*/

#include <stdio.h>
#include "timeutils.h"
#include "neo7m.h"
#include "nextion.h"
#include "dds.h"


#define TIMED true
#define RANDOM false

void displayclock()
{
	unsigned char est;
	static uint8_t last[6] = {0xff,0xff,0xff,0xff,0xff,0xff};		// last time cache

	est = (NavPvt.hour + 10) % 24;

	if (last[0] != est/10)		// hour
	{
		setndig("top.n1",est/10);
		last[0] = est/10;
	}
	if (last[1] != est/10)		// hour
	{
		setndig("top.n2",est%10);
		last[1] = est%10;
	}
	if (last[2] != NavPvt.min/10)		// min
	{
		setndig("top.n3",NavPvt.min/10);
		last[2] = NavPvt.min/10;
	}
	if (last[3] != NavPvt.min%10)		// min
	{
		setndig("top.n4",NavPvt.min%10);
		last[3] = NavPvt.min%10;
	}
	if (last[4] != NavPvt.sec/10)		// sec
	{
		setndig("top.n5",NavPvt.sec/10);
		last[4] = NavPvt.sec/10;
	}
	if (last[5] != NavPvt.sec%10)		// sec
	{
		setndig("top.n6",NavPvt.sec%10);
		last[5] = NavPvt.sec%10;
	}
}

// populate the gps screen
void drawgps()
{
char buffer[64];
char *valstr;
signed int latdeg, londeg, scale;
signed long latfrac, lonfrac;
unsigned long lat, lon;
unsigned char trk;
unsigned int col;

char *flagstr[6] = {"Not Power Save Mode","Enabled not acquired","Acquisition","Tracking","Power Optimised Tracking","Inactive"};
char *tracking[2] = {"No FIX","Valid FIX"};
	
	switch (NavPvt.valid & 0x07)
	{
		case 7:
			valstr = "UTC locked";
			break;
		default:
			valstr = "UTC acquiring";
			break;
	}

	trk = (NavPvt.flags & 1);			// Fix flag

	sprintf(buffer,"%s, %s, %s",valstr,flagstr[NavPvt.flags >> 2],tracking[trk]);	
	setlcdtext("gps.g0.txt",buffer);

	lat = (NavPvt.lat < 0L) ? -NavPvt.lat : NavPvt.lat;
	lon = (NavPvt.lon < 0L) ? -NavPvt.lon : NavPvt.lon;

	latdeg = lat / 10000000L;
	londeg = lon / 10000000L;

	latfrac = lat - (latdeg * 10000000L);
	lonfrac = lon - (londeg * 10000000L);

	if (NavPvt.lat < 0L)
		latdeg = -latdeg;
	if (NavPvt.lon < 0L)
		londeg = -londeg;

	sprintf(buffer,"%d.%lu %d.%lu",latdeg,latfrac,londeg,lonfrac);
	setlcdtext("gps.t1.txt",buffer);

	sprintf(buffer,"%lum",NavPvt.height/1000);
	setlcdtext("gps.t4.txt",buffer);

	sprintf(buffer,"Sats: %02u",NavPvt.numSV);
	setlcdtext("gps.t2.txt",buffer);

	col = NEX_CRED;
	if (NavPvt.numSV > 9)
	col = NEX_CYELLOW;
	else
	if (NavPvt.numSV > 3)
		col = NEX_CYELLOW;

	sprintf(buffer,"%u",col);
	setlcdnum("gps.j0.pco",buffer);

	scale = (NavPvt.numSV<<2)+(NavPvt.numSV<<1);		// for the bargraph
	if (scale > 255)
		scale = 255;
	sprintf(buffer,"%d",(uint8_t)scale);
	setlcdnum("gps.j0.val",buffer);

	sprintf(buffer,"%du",(unsigned)NavPvt.tAcc & 0xffff);
	setlcdtext("gps.t3.txt",buffer);

}

// populate the ocxo screen
void drawocxo()
{
	// setlcdtext("gps.t0.txt","hello world");
}


// populate the dds top screen
void drawddstop()
{
	// setlcdtext("gps.t0.txt","hello world");
}

// populate the dds keypad screen
void ddskeypad()
{
	// setlcdtext("gps.t0.txt","hello world");
}

// populate the debug screen
void drawdebug()
{
	// setlcdtext("gps.t0.txt","hello world");
}

// read the dds frequency variable from the LCD
// store the result in the global
void getddsfreq()
{
	uint8_t result;

	result = getlcdnvar("dds.ddsfreq.val",&ddsfreq);
	if (result == NEX_ENUM)
	{
		printf("dds frequency = %lu\n\r",ddsfreq);
	}
	else
	{
		printf("dds frequency not found\n\r");
	}
}


// comes here on a lcd touch
void lcdtouch()
{
	lcdtouched = 0;		// inactive
	printf("lcdtouched: page %d button %d, %02x\n\r",lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);

	if ((lcdrxbuffer[1] == 4) && (lcdrxbuffer[2] == 13))		// DDS main page, the enter button
	{
		getddsfreq();
	}
}


// comes here on a lcd page event
// or a timed refresh event
void lcdpageevent(bool timed)
{
	lcdpevent = 0;		// inactive

//	printf("lcdpage: page %d\n\r",pagenum);

	switch(pagenum)
	{
		case 0:		// Top screen
			displayclock();
		break;

		case 1:		// GPS screen
			drawgps();
		break;

		case 2:		// OCXO screen
			drawocxo();
		break;

		case 3:		// DDS top menu
			drawddstop();
		break;

		case 4:		// DDS keypad
			ddskeypad();
		break;

		case 5:		// DDS up/down screen
			getddsfreq();	
		break;

		case 6:		// debug screen
			drawdebug();
		break;

		default:
			printf("invalid lcd page sent\n\r");
		break;

	}
}


	// This is the LCD  ladder to control
	// User input and display output
	// returns current LCD page
	void ladder(void)
	{
		if (lcdtouched == 0xff)		// a touch event to deal with
		{
			lcdtouch();
		}

		if (lcdpevent == 0xff)	// lcd sent a page event
		{
			lcdpageevent(RANDOM);
		}

		if (timer3 == 0)		// timeout
		{
			lcdpageevent(TIMED);		// so refresh the page anyway
			settimer3(200/4);
		}
		return;
	}
