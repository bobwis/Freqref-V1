/*
* ladder.c
*
* Created: 22/09/2017 12:02:12 PM
*  Author: bob
*/

#include "timeutils.h"
#include "neo7m.h"
#include "nextion.h"
#include <stdio.h>

#define TOP	0
// This is the LCD state machine ladder to control
// User input and display output
void ladder(void)
{
	static uint8_t pstate = 0;			// page state variable

	pstate = getlcdpage();		// get current page

	if (lcdevent == 0xff)		// some event to deal with
	{
		switch(pstate) {

			// lcd page 0 (top)
			case 0:
			//		printf("state %d\n\r",pstate);
			if (lcdstatus == 0x65)		// button pressed
			{
				printf("pstate %d, page %d button %d, %02x\n\r",pstate,lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);
			}
			break;

			// lcd page 1 gps status
			case 1:
			if (lcdstatus == 0x65)		// button pressed
			{
				printf("pstate %d, page %d button %d, %02x\n\r",pstate,lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);
			}
			break;

			// lcd page 2 ocxo status
			case 2:
			if (lcdstatus == 0x65)		// button pressed
			{
				printf("pstate %d, page %d button %d, %02x\n\r",pstate,lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);
			}
			break;

			// lcd page 3 dds
			case 3:
			if (lcdstatus == 0x65)		// button pressed
			{
				printf("pstate %d, page %d button %d, %02x\n\r",pstate,lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);
			}
			break;

			// lcd page 4 fpad
			case 4:
			if (lcdstatus == 0x65)		// button pressed
			{
				printf("pstate %d, page %d button %d, %02x\n\r",pstate,lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);
			}
			break;

			// lcd page 5 fadj
			case 5:
			if (lcdstatus == 0x65)		// button pressed
			{
				printf("pstate %d, page %d button %d, %02x\n\r",pstate,lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);
			}
			else
			if (lcdstatus == 0x66)		// page index sent - means updated
			{
				printf("pstate %d, page %d pageid %d, %02x\n\r",pstate,lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);
			}
			break;

			// lcd page 6 debug
			case 6:
			if (lcdstatus == 0x65)		// button pressed
			{
				printf("pstate %d, page %d button %d, %02x\n\r",pstate,lcdrxbuffer[1],lcdrxbuffer[2],lcdrxbuffer[3]);
			}
			break;

			default:
			printf("pstate default 0x%02x\n\r",pstate);
			break;
		}
		lcdevent = 0;		// inactive
	}
}