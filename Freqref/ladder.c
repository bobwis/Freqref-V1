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
char *flagstr[6] = {"Power Save","Enabled not acquired","Acquisition","Tracking","Power Optimised Tracking","Inactive"};

	
	switch (NavPvt.valid & 0x07)
	{
		case 7:
			valstr = "UTC locked";
			break;
		default:
			valstr = "UTC acquiring";
			break;
	}
	sprintf(buffer,"%s, %s",valstr,flagstr[NavPvt.flags >> 2]);	
	setlcdtext("gps.t0.txt",buffer);
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
