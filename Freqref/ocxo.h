/*
 * ocxo.h
 *
 * Created: 26/09/2017 7:55:07 AM
 *  Author: bob
 */ 


#ifndef OCXO_H_
#define OCXO_H_

#define CSDAC  47	// PL2 DAC  MP4882XP DUAL DAC

#define low false
#define high true

#define REFVAL 1000 // 2818  // 2820	// DAC approx 10MHz reference level (EEPROM eventually)
#define MAXCNT 420000	// close to 2^32, the size of the 32 bit ocxo and gps counters
#define WARMINGTIME 18	// arbitary warm up time allowance

extern unsigned long ocxocount, gpscount, ocxointerval;
extern unsigned int dacval;
extern bool ocxounlock;
extern unsigned long hotstarttime;

// init 
bool ocxoinit(void);

// reset frequency counters
void resetcnt(void);

// proportional control
void propocxo();

void setdacandwait(int,int);

/*
Read the 4 bytes = 32 bits out of each counter
assumes count is already latched
*/
unsigned long read32cnt(int);

// transfer the current counts to holding registers
void capturecnt(void);

#endif /* OCXO_H_ */