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
		updategps();
		if (fastmsectime() > (now + 10000/4))		// timeout 5 secs
		{
			now = fastmsectime();
			setndig("n1",realPacket.hour/10);
			setndig("n2",realPacket.hour%10);
			setndig("n3",realPacket.min/10);
			setndig("n4",realPacket.min%10);
		}
	}

	printf("Goodbye World\n\r");
	/* Replace with your application code */

}
