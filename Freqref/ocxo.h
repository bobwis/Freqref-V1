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

#define REFVAL 2576	// DAC approx 10MHz reference level

extern unsigned long ocxocount, gpscount, ocxointerval;
extern unsigned int dacval;
extern bool ocxounlock;

// init 
bool ocxoinit(void);

// reset frequency counters
void resetcnt(void);

// proportional control
void propocxo();

#endif /* OCXO_H_ */