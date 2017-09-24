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
volatile uint8_t pagenum = 0;		// binary LCD page number 

unsigned char lcdrxbuffer[32] = {""};
volatile uint8_t lcdstatus = 0;		// response code, set to 0xff for not set
volatile uint8_t lcdtouched = 0;		// this gets set to 0xff when an autonomous event or cmd reply happens
volatile uint8_t lcdpevent = 0;		// lcd reported a page. set to 0xff for new report

// send a string to the LCD (len max 255)
void writelcd(char *str)
{
	unsigned char i = 0;
	
	while (str[i])
	{
		//USART_3_write(str[i]);
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
	lcdstatus = 0xff;			// arm response variable - 0xff is unused status code

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
// assumes 	lcdstatus = 0xff prior to call	
unsigned char getlcdack()
{
	settimer2(1000/4);
//	printf("waiting for lcd status\n\r");
	while((lcdstatus == 0xff) && (timer2))
	{
		;  // wait for lcd rx interrupt service to get something
	}
	if (lcdstatus == 0xff)
	{
		printf("Timeout waiting for LCD response\n\r");
	}
#if 0
	else
	{
		printf("lcd status %02x\n\r",lcdstatus);
	}
#endif
	return(lcdstatus);
}

// read a lcd named variable (unsigned long) expects numeric value
// return -1 for error
 char getlcdnvar(char * id, unsigned long *data)
{
	writelcd("get ");
	writelcdcmd(id);
	if (getlcdack() == 0xff)			// wait for response
	{
		printf("No response from getlcdnvar cmd\n\r");
	}
	if (lcdrxbuffer[0] == NEX_ENUM)		// numeric response: 4 bytes following
	{
		*data = *(unsigned long *)&lcdrxbuffer[1];
//		printf("getlcdnvar returned %li\n\r",*data);
		return(lcdrxbuffer[0]);
	}
	else
	{
		if (lcdrxbuffer[0] == NEX_SVAR)		// Variable Name Invalid
		{
				printf("getlcdnvar: var name %s invalid\n\r",id);
		}
	}
	return(-1);
}


// send some text to a lcd text object
void setlcdtext(char id[], char string[])
{
	writelcd(id);
	writelcd("=\"");
	writelcd(string);
	writelcdcmd("\"");
}

// send some numbers to a lcd obj.val object
void setlcdnum(char id[], char string[])
{
//printf("%s=%s",id,string);
	writelcd(id);
	writelcd("=");
	writelcdcmd(string);
}

// find the current lcd page (char as int8)
// this is processed mostly by the ISR, this routine assumes success
// return -1 for error
uint8_t getlcdpage()
{
printf("getpg\n\r");
	writelcdcmd("sendme");
	if (getlcdack() == 0xff)			// wait for response
	{
		printf("No response from getlcdpage cmd\n\r");
	}
	return(pagenum);
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
			getlcdack();
			fastdelay_ms(1000/4);	// wait for page to activate
			pagenum = getlcdpage();		// associate with its number
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
	while (USART_2_is_rx_ready())		// data in the rx buffer
	{
		if(isnexpkt(lcdrxbuffer,sizeof(lcdrxbuffer)) == -1)
		{
			return;		// nothing found
		}
		else // got a packet
		{
			lcdstatus = lcdrxbuffer[0];
			if ((lcdrxbuffer[0] >= NEX_SINV) && (lcdrxbuffer[0] <= NEX_SLEN))	// a status code packet - eg error
			{
				lcdtouched = 0;
				lcdpevent = 0;
				printf("Err? LCD 0x%02x\n\r",lcdrxbuffer[0]);
			}
			else  // this is either a touch event or a response to a query packet
			{
				if (lcdrxbuffer[0] == NEX_ETOUCH)
				{
					lcdtouched = 0xff;		// its a touch
					lcdpevent = 0;
				}
				else  
				if (lcdrxbuffer[0] == NEX_EPAGE)		// its either an autonomous or requested current page response
				{
					lcdtouched = 0;
					lcdpevent = 0xff;		// notify lcd page event happened
					pagenum = lcdrxbuffer[1];
				}
			}
		}
	}
}