#include <atmel_start.h>
#include "timeutils.h"
#include "neo7m.h"
#include "nextion.h"
#include <stdio.h>


int main(void)
{
	volatile int i;
	unsigned int now;
	volatile unsigned char ch;

	now = fastmsectime();

	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	sei();

	printf("Hello World\n\r");

	fastdelay_ms(5000/4);

	setupneo();
	printf("Neo7 setup returned\n\r");

	now = fastmsectime();
	while(1)
	{

		//		decodelcd();		// lcd test and debug
		// Update RealPacket

		if (fastmsectime() > (now + 990/4))		// timeout almost 1 sec
		{
			now = fastmsectime();
			setndig("n1",NavPvt.hour/10);
			setndig("n2",NavPvt.hour%10);
			setndig("n3",NavPvt.min/10);
			setndig("n4",NavPvt.min%10);
			setndig("n5",NavPvt.sec/10);
			setndig("n6",NavPvt.sec%10);
#define PRINTDEBUG
#ifdef PRINTDEBUG
			printf("Date  %d %02d %02d  ", NavPvt.day, NavPvt.month,  NavPvt.year);
			printf("Time %02d:%02d:%02d  UTC     Epoch  %lu\r\n", NavPvt.hour, NavPvt.min,  NavPvt.sec,NavPvt.iTOW);
#endif
		}
	}

	printf("Goodbye World\n\r");
	/* Replace with your application code */

}
