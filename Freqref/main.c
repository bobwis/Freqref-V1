#include <atmel_start.h>
#include <stdio.h>
#include "timeutils.h"
#include "neo7m.h"
#include "nextion.h"
#include "ladder.h"
#include "dds.h"


void displayclock()
{
	unsigned char est;
	static uint8_t last[6] = {0xff,0xff,0xff,0xff,0xff,0xff};		// last time cache

	est = (NavPvt.hour + 10) % 24;

	if (last[0] != est/10)		// hour
	{
		setndig("top.n1",est/10);
		last[0] = est/10;
	}
	if (last[1] != est/10)		// hour
	{
		setndig("top.n2",est%10);
		last[1] = est%10;
	}
	if (last[2] != NavPvt.min/10)		// min
	{
		setndig("top.n3",NavPvt.min/10);
		last[2] = NavPvt.min/10;
	}
	if (last[3] != NavPvt.min%10)		// min
	{
		setndig("top.n4",NavPvt.min%10);
		last[3] = NavPvt.min%10;
	}
	if (last[4] != NavPvt.sec/10)		// sec
	{
		setndig("top.n5",NavPvt.sec/10);
		last[4] = NavPvt.sec/10;
	}
	if (last[5] != NavPvt.sec%10)		// sec
	{
		setndig("top.n6",NavPvt.sec%10);
		last[5] = NavPvt.sec%10;
	}
}

int main(void)
{
	uint8_t pageindex;
	char result;

	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	sei();

	printf("----------------------------\n\r");
	printf("Hello World\n\r");
	setupneo();
//	printf("Neo7 setup returned\n\r");

	setlcdpage("top",true);
	pageindex = getlcdpage();		// get current page

	// main loop to process all the modules

	result = getlcdnvar("dds.ddsfreq.val",&ddsfreq);
	if (result == NEX_ENUM)
	{
		printf("dds frequency = %ld\n\r",ddsfreq);
	}
	else
	{
		printf("dds frequency not found\n\r");
	}

	settimer3(200/4);
	while(1)
	{
		if (timer3 == 0)		// timeout 
		{
			settimer3(200/4);
			if (pageindex == 0)
			{
				displayclock();
			}
		}
		pageindex = ladder();
	}

	printf("Goodbye World\n\r");
	/* Replace with your application code */
}
