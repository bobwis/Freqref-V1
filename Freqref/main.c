#include <atmel_start.h>
#include <stdio.h>
#include "timeutils.h"
#include "neo7m.h"
#include "nextion.h"
#include "ladder.h"
#include "dds.h"
#include "ocxo.h"
#include "pid.h"
#include "ad9910.h"

int main(void)
{
	char result;
	uint32_t freq;

	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	sei();

	printf("----------------------------\n\r");
	printf("Hello World\n\r");
	setupneo();
	printf("Neo7 setup returned\n\r");

	writelcdcmd("");		// null cmd to clear any partial cmd
	setlcdpage("top",true);

	printf("setup OCXO\n\r");
	if (!(ocxoinit()))
	{
		printf("ocxo init failed\n\r");
	}

	result = getlcdnvar("dds.ddsfreq.val",&freq);
	if (result == NEX_ENUM)
	{
		printf("dds frequency = %lu\n\r",freq*10L);
	}
	else
	{
		printf("dds frequency not found\n\r");
		freq = 15000000L;
	}
//		siswiz();
		ddsfreq = freq*10L;
		si5351aSetFrequency(ddsfreq);

//AD9910_reset();
//		AD9910_init();
//		delay_ms(3000);
//        AD9910_single_tone(0x3FFF, 0, 100000000L, 0);

	settimer3(200/4);
	while(1)
	{
		//setdacandwait(4095);
		// debugdac();
		//		pid();
		//		propocxo(); // proxess the ocxo tracking
		track3ocxo(); // process the ocxo tracking
		//		testocxo();
		//		testglitch();
		ladder();  	// main loop to process all the modules
	}

	printf("Goodbye World\n\r");
	/* Replace with your application code */
}
