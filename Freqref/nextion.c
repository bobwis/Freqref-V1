/*
* nextion.c
*
* Created: 15/09/2017 4:36:32 PM
*  Author: bob
*/

// Nextion LCD utilities
// Nextion on Arduino MEGA USART 2

#include "timeutils.h"
#include <stdio.h>


// try to get a single message packet from the LCD
// returns length of packet or -1 if timeout
int getlcdpkt(unsigned char buffer[])
{
	volatile int termcnt, i;
	unsigned int now;
	volatile unsigned char ch;

	now = fastmsectime();
	termcnt = 0;
	i = 0;
	while(termcnt < 3)
	{
		if (USART_2_is_rx_ready())
		{
			now = fastmsectime();
			ch = USART_2_read();
			buffer[i++] = ch;
			if (ch == 0xff)
			{
				termcnt++;
			}
			else
			{
				termcnt=0;
			}
		}
		if (fastmsectime() > (now + 5000/4))		// timeout 5 secs
		return(-1);
	}
	return(i);
}

// write a number digit on the LCD to a num object
void setndig(char *id, uint8_t val)
{
	int i;
	char msg[16];
	
	sprintf(msg,"%s.val=%1d\xff\xff\xff",id,val);
	for (i=0; msg[i]; i++)
	{
		USART_2_write(msg[i]);
	}
}

void decodelcd()
{
	volatile int i, j;
	unsigned char buffer[64];


	j = -1;
	while (j)
	{
		j = getlcdpkt(buffer);
		if (j > 0)				// we got something valid
		{
			for(i=0; i<j; i++)
			{
				printf("0x%02x ",buffer[i]);
			}
			printf("\n\r");
		}
#if 0
		else
		{
			printf("Still waiting for LCD press\n\r");
		}
#else
	return;
#endif
	}
}

