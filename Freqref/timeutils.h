/*
 * timeutils.h
 *
 * Created: 13/09/2017 12:20:21 PM
 *  Author: bob
 */ 


#ifndef TIMEUTILS_H_
#define TIMEUTILS_H_

extern volatile uint16_t m4sectimer;		// global 4.096mS tick count (16 bits)
extern volatile uint64_t m1sectimer;		// global 1mS tick count (64 bits)


// read the 64 bit mSec counter
extern uint64_t msectime(void);


// read the 16 bit 4mSec counter
extern uint16_t fastmsectime(void);


// Uses Hardware timer 5 which is set to 1mS interrupt
// delay will be 0 < 1mSec for parameter of 1, 1mS < 2mS for parameter of 2 etc
extern void delay_ms(uint16_t count);


// Uses Hardware timer 4 which is set to 4.096mS interrupt
// delay will be 0 < 4.096mS for parameter of 1
extern void fastdelay_ms(uint16_t count);


#endif /* TIMEUTILS_H_ */