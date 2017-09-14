#include <atmel_start.h>
#include "timeutils.h"
#include "neo7m.h"
#include <stdio.h>


int main(void)
{
	volatile int i;

	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	sei();

	printf("Hello World\n\r");

	fastdelay_ms(5000/4);

	setupneo();
	printf("Neo7 setup returnedd\n\r");
	while(1)
	{
		rdgps();
	}

	printf("Goodbye World\n\r");
	/* Replace with your application code */

}
