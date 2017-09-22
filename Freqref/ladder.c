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
	static uint8_t state = 0;			// state variable, tens is current LCD pageno

	switch(state) {
// lcd page 0 (top)
		case 0:
		printf("state %d\n\r",state);
		break;


// lcd page 1 gps status
		case 10:
		state = 0;
		printf("state %d\n\r",state);
		break;



		default:
		printf("state default\n\r");
		break;
	}
}