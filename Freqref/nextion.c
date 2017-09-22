/*
* nextion.c
*
* Created: 15/09/2017 4:36:32 PM
*  Author: bob
*/

// Nextion LCD utilities
// Nextion on Arduino MEGA USART 2


#include <stdio.h>
#include <string.h>
#include "timeutils.h"
#include "nextion.h"


char currentpage[16] = {""};
unsigned char lcdrxbuffer[32] = {""};
volatile uint8_t lcdstatus = 0;
volatile uint8_t lcdevent = 0;


// send a string to the LCD (len max 255)
void writelcd(char *str)
{
	unsigned char i = 0;
	
	while (str[i])
	{
		//		USART_3_write(str[i]);
		USART_2_write(str[i++]);
	}
}

// send a string to the LCD (len max 255)
// terminate with three 0xff's
void writelcdcmd(char *str)
{
	char i = 0;
	
	writelcd(str);
	for (i=0; i<3; i++)
	{
		USART_2_write(0xff);
	}

}

// write a number digit on the LCD to a num object
void setndig(char *id, uint8_t val)
{
	char msg[32];
	
	sprintf(msg,"%s.val=%1d",id,val);
	writelcdcmd(msg);
}


// read the response to a command from the lcd
// returns 0xff on timeout, busy wait
unsigned char getlcdack()
{
	int result;
	char buffer[8];
	unsigned int now;

	lcdstatus = 0xff;
	now = fastmsectime();
//	printf("waiting for lcd status\n\r");
	while((lcdstatus == 0xff) && (fastmsectime() < (now + (1000/4))))
	{
		;// wait for lcd rx interrupt service to get something
	}
	if (lcdstatus == 0xff)
	{
		printf("Timeout waiting for LCD response\n\r");
	}
	else
	{
//		printf("lcd status %02x\n\r",lcdstatus);
	}
	return(lcdstatus);
}

// find the current lcd page (char as int8)
// return -1 for error
char getlcdpage()
{
	unsigned char buffer[16] = "";
	unsigned char j;

	writelcdcmd("sendme");
	if (getlcdack() == 0xff)			// wait for response
	{

	}
	if (lcdrxbuffer[0] == 0x66)		// 'page id' response
	{
//		printf("we are on page %i\n\r",lcdrxbuffer[1]);
		return(lcdrxbuffer[1]);
	}
	return(-1);
}

// display a chosen page
// use cached pagename to skip if matched current page unless forced
// returns lcd response code
void setlcdpage(char *pagename, bool force)
{
	if (*pagename)		// not null
	{
		if (!(strncmp(pagename,currentpage,sizeof(currentpage))) || (force))		// already on this page
		{
			writelcd("page ");
			writelcdcmd(pagename);
			strncpy(currentpage,pagename,sizeof(currentpage));
		}
	}
	return;
}

// Check if this is an LCD packet (called via ISR)
// try to get a single message packet from the LCD
// returns packet
int isnexpkt(unsigned char buffer[],uint8_t size)
{
	static uint8_t termcnt = 0;
	static uint8_t i = 0;
	int index;
	unsigned char ch;


	ch = USART_2_read();
	buffer[i++] = ch;
	if (ch == 0xff)
	{
		termcnt++;
		if (termcnt == 3)
		{
			index = i;
			i = 0;
			termcnt = 0;
			return(index);
		}
	}
	else
	{
		termcnt = 0;
	}

	if (i == size)	// overrun
	{
		i = 0;
		termcnt = 0;
	}
	return(-1);
}

// Read chars from the LCD port
// Try to build a packet
// If its a status, partially decode it and set global LCD last returned status
// **** NOTE ****  This is called from within Timer 4 Interrupt Service Routine every 4.096mS
extern void processnex(void)
{
	unsigned char data;

	while (USART_2_is_rx_ready())		// data in the rx buffer
	{
		if(isnexpkt(lcdrxbuffer,sizeof(lcdrxbuffer)) == -1)
		{
			return;		// nothing found
		}
		else // got a packet
		{
			lcdstatus = lcdrxbuffer[0];
			if ((lcdrxbuffer[0] >= NEX_SINV) && (lcdrxbuffer[0] < NEX_SLEN))	// a status code packet
			{
				lcdevent = 0;
				printf("LCDstat %02x\n\r",lcdrxbuffer[0]);
			}
			else  // this is an 'touch event' type of packet
			{
				lcdevent = 0xff;
				if (lcdrxbuffer[0] != 0x66)
					printf("LCDtouch %02x\n\r",lcdrxbuffer[0]);
			}
		}
	}

}