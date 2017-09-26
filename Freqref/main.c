#include <atmel_start.h>
#include <stdio.h>
#include "timeutils.h"
#include "neo7m.h"
#include "nextion.h"
#include "ladder.h"
#include "dds.h"
#include "ocxo.h"


int main(void)
{
	char result;

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

	result = getlcdnvar("dds.ddsfreq.val",&ddsfreq);
	if (result == NEX_ENUM)
	{
		printf("dds frequency = %lu\n\r",ddsfreq);
	}
	else
	{
		printf("dds frequency not found\n\r");
	}

	settimer3(200/4);
	while(1)
	{
		propocxo(); // proxess the ocxo tracking
		ladder();  	// main loop to process all the modules
	}

	printf("Goodbye World\n\r");
	/* Replace with your application code */
}
