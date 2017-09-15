/*
 * ubxgps.c
 *
 * Created: 15/09/2017 9:21:15 AM
 *  Author: paul
 */ 


// defines 

void   (*gpsPortPrintBinary)(uint8_t *buf, uint16_t length);
enum	ubloxState { WAIT_SYNC1, WAIT_SYNC2, GET_CLASS, GET_ID, GET_LL, GET_LH, GET_DATA, GET_CKA, GET_CKB  } ubloxProcessDataState;
uint8_t  (*gpsPortRead)(void);
void     (*gpsPortClearBuffer)(void);
uint16_t (*gpsPortNumCharsAvailable)(void);
