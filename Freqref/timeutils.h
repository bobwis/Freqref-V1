/*
 * timeutils.h
 *
 * Created: 13/09/2017 12:20:21 PM
 *  Author: bob
 */ 

#include <atmel_start.h>
#ifndef TIMEUTILS_H_
#define TIMEUTILS_H_

extern volatile uint16_t timer1;		// global 4.096mS tick down count (16 bits) used in fastdelay_ms(uint16_t count)
extern volatile uint16_t timer2;		// global 4.096mS tick down count (16 bits) used in lcd
extern volatile uint16_t timer3;		// global 4.096mS tick down count (16 bits) used in main
extern volatile uint64_t m1sectimer;		// global 1mS tick up count (64 bits)


// read the 64 bit mSec counter
extern uint64_t msectime(void);

// Uses Hardware timer 5 which is set to 1mS interrupt
// delay will be 0 < 1mSec for parameter of 1, 1mS < 2mS for parameter of 2 etc
extern void delay_ms(uint16_t count);


// Uses Hardware timer 4 which is set to 4.096mS interrupt
// delay will be 0 < 4.096mS for parameter of 1
extern void fastdelay_ms(uint16_t count);

// set software timer 2
extern void settimer2(uint16_t);

// set software timer 3
extern void settimer3(uint16_t);

#endif /* TIMEUTILS_H_ */