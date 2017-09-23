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



// read the dds frequency variable from the LCD
// store the result in the global
void getddsfreq()
{
	uint8_t result;

	result = getlcdnvar("dds.ddsfreq.val",&ddsfreq);
	if (result == NEX_ENUM)
	{
		printf("dds frequency = %ld\n\r",ddsfreq);
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

// This is the LCD  ladder to control
// User input and display output
// returns current LCD page
uint8_t ladder(void)
{
	static uint8_t pstate = 0;			// page state variable

	pstate = pagenum;		// get current page

	if (lcdtouched == 0xff)		// a touch event to deal with
	{
		lcdtouch();
	}

	if (lcdpevent == 0xff)	// lcd sent a page event
	{
		lcdpevent = 0;		// inactive
		printf("pevent: %d, page %d\n\r",pstate,lcdrxbuffer[1]);

		if (lcdrxbuffer[1] == 5)		// DDS Up/Down screen
		{
			getddsfreq();				// probably changing it
		}
	}
	return(pstate);
}