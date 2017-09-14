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

 // read the 16 bit 4mSec counter
 inline uint16_t fastmsectime(void)
 {
	 uint16_t currentms;
	 ENTER_CRITICAL(W);
	 currentms = m4sectimer;
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
 void fastdelay_ms(uint16_t count)
 {
	 volatile uint16_t k,j;
	 while(1) {
		 j = fastmsectime();
		 k = j + (uint16_t)count;
		 while(1) {
			 j = fastmsectime();
			 if (j >= k)
			 {
				 return;
			 }
		 }
	 }
 }