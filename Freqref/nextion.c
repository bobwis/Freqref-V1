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

int getlcdpkt(unsigned char buffer[])
{
	int termcnt = 0, i = 0, timeout;
	unsigned char ch;

	timeout = fastmsectime();
	while(termcnt < 3)
	{
		if (USART_2_is_rx_ready())
		{
			timeout = fastmsectime();
			ch = USART_2_read();
			buffer[i++] = ch;
			if (ch == 0xff)
			{
				termcnt++;
			}
		}
		if ((timeout + (500/4)) > fastmsectime())		// 0.5 second
		return(-1);
	}
	return(i);
}

void decodelcd()
{
	int i, j;
	unsigned char buffer[64];

	j = -1;
	while (j)
	{
		j = getlcdpkt(buffer);
		if (j > 0)
		{
			for(i=0; i<j; i++)
			{
				printf("0x%02x",buffer[i]);
			}
			printf("\n\r");
		}
	}
}