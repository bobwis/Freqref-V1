/*
 * nextion.h
 *
 * Created: 15/09/2017 4:36:54 PM
 *  Author: bob
 */ 
 // Nextion LCD definitions

#ifndef NEXTION_H_
#define NEXTION_H_

// try to get one packet from the LCD
void decodelcd(void);

// write a number digit on the LCD to a num object
void setndig(char *, int);


#endif /* NEXTION_H_ */