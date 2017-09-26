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

extern unsigned long ocxocount, gpscount, ocxointerval;
extern unsigned int adcval;
extern bool ocxounlock;

// init 
bool ocxoinit(void);


#endif /* OCXO_H_ */