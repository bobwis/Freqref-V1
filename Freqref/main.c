#include <atmel_start.h>
#include "timeutils.h"
#include "neo7m.h"
#include "nextion.h"
#include <stdio.h>

void displayclock()
{
	unsigned char est;

	est = (NavPvt.hour + 10) % 24;
	setndig("top.n1",est/10);
	setndig("top.n2",est%10);
	setndig("top.n3",NavPvt.min/10);
	setndig("top.n4",NavPvt.min%10);
	setndig("top.n5",NavPvt.sec/10);
	setndig("top.n6",NavPvt.sec%10);

}

int main(void)
{
	volatile int i;
	unsigned int now;
	volatile unsigned char ch;
	int pageindex;

	char * pages[7] ={"0", "1", "ocxo", "3", "4", "5", "6"};

	now = fastmsectime();

	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	sei();

	printf("Hello World\n\r");
	setupneo();
	printf("Neo7 setup returned\n\r");

	setlcdpage("top",true);
	pageindex = 0;

	// main loop to process all the modules
	now = fastmsectime();

	while(1)
	{
		if (fastmsectime() > (now + 500/4))		// timeout 0.5 sec
		{
			now = fastmsectime();
			if (pageindex == 0)
			{
				displayclock();
			}
		}
		ladder();
	}

	printf("Goodbye World\n\r");
	/* Replace with your application code */

}
