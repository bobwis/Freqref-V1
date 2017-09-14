/**
 * \file
 *
 * \brief TC16 related functionality implementation.
 *
 * Copyright (C) 2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include <tc16.h>
#include <utils.h>

/**
 * \brief Initialize TIMER_0 interface
 */
int8_t TIMER_0_init()
{

	/* Enable TC4 */
	PRR1 &= ~(1 << PRTIM4);

	// TCCR4A = (0 << COM4A1) | (0 << COM4A0) /* Normal port operation, OCA disconnected */
	//		 | (0 << COM4B1) | (0 << COM4B0) /* Normal port operation, OCB disconnected */
	//		 | (0 << WGM41) | (0 << WGM40); /* TC16 Mode 0 Normal */

	TCCR4B = (0 << WGM43) | (0 << WGM42)                /* TC16 Mode 0 Normal */
	         | 0 << ICNC4                               /* Input Capture Noise Canceler: disabled */
	         | 0 << ICES4                               /* Input Capture Edge Select: disabled */
	         | (0 << CS42) | (0 << CS41) | (1 << CS40); /* No prescaling */

	// ICR4 = 0; /* Input capture value, used as top counter value in some modes: 0 */

	// OCR4A = 0; /* Output compare A: 0 */

	// OCR4B = 0; /* Output compare B: 0 */

	TIMSK4 = 0 << OCIE4B   /* Output Compare B Match Interrupt Enable: disabled */
	         | 1 << OCIE4A /* Output Compare A Match Interrupt Enable: enabled */
	         | 0 << ICIE4  /* Input Capture Interrupt Enable: disabled */
	         | 0 << TOIE4; /* Overflow Interrupt Enable: disabled */

	return 0;
}

/**
 * \brief Initialize TIMER_1 interface
 */
int8_t TIMER_1_init()
{

	/* Enable TC5 */
	PRR1 &= ~(1 << PRTIM5);

	// TCCR5A = (0 << COM5A1) | (0 << COM5A0) /* Normal port operation, OCA disconnected */
	//		 | (0 << COM5B1) | (0 << COM5B0) /* Normal port operation, OCB disconnected */
	//		 | (0 << WGM51) | (0 << WGM50); /* TC16 Mode 0 Normal */

	TCCR5B = (0 << WGM53) | (0 << WGM52)                /* TC16 Mode 0 Normal */
	         | 0 << ICNC5                               /* Input Capture Noise Canceler: disabled */
	         | 0 << ICES5                               /* Input Capture Edge Select: disabled */
	         | (0 << CS52) | (0 << CS51) | (1 << CS50); /* No prescaling */

	// ICR5 = 0; /* Input capture value, used as top counter value in some modes: 0 */

	OCR5A = 16000; /* Output compare A: 16000 */

	// OCR5B = 0; /* Output compare B: 0 */

	TIMSK5 = 0 << OCIE5B   /* Output Compare B Match Interrupt Enable: disabled */
	         | 1 << OCIE5A /* Output Compare A Match Interrupt Enable: enabled */
	         | 0 << ICIE5  /* Input Capture Interrupt Enable: disabled */
	         | 0 << TOIE5; /* Overflow Interrupt Enable: disabled */

	return 0;
}
