/*
* CFile1.c
*
* Created: 13/09/2017 12:18:24 PM
*  Author: bob
*/
#include <atmel_start.h>
#include <atomic.h>
#include "timeutils.h"

// read the 64 bit mSec counter
inline uint64_t msectime(void)
{
	uint64_t currentms;
	ENTER_CRITICAL(W);
	currentms = m1sectimer;
	EXIT_CRITICAL(W);
	return(currentms);
}


// Uses Hardware timer 5 which is set to 1mS interrupt
// delay will be 0 < 1mSec for parameter of 1, 1mS < 2mS for parameter of 2 etc
void delay_ms(uint16_t count)
{
	volatile uint64_t k,j;
	while(1) {
		j = msectime();
		k = j + (uint64_t)count;
		while(1) {
			j = msectime();
			if (j >= k)
			{
				return;
			}
		}
	}
}

// Uses Hardware timer 4 which is set to 4.096mS interrupt
// delay will be 0 < 4.096mS for parameter of 1
// users software countdown timer1
void fastdelay_ms(uint16_t count)
{
	ENTER_CRITICAL(W);
	timer1 = count;
	EXIT_CRITICAL(W);
	while(timer1)  // big assumption that no irq mask required for this test
	{
		; // wait busy
	}
}

// Uses Hardware timer 4 which is set to 4.096mS interrupt
// delay will be 0 < 4.096mS for parameter of 1
// users software countdown timer2
inline void settimer2(uint16_t count)
{

	ENTER_CRITICAL(W);
	timer2 = count;		// approximate 4msec steps
	EXIT_CRITICAL(W);
}

// Uses Hardware timer 4 which is set to 4.096mS interrupt
// delay will be 0 < 4.096mS for parameter of 1
// users software countdown timer3
inline void settimer3(uint16_t count)
{

	ENTER_CRITICAL(W);
	timer3 = count;		// approximate 4msec steps
	EXIT_CRITICAL(W);
}