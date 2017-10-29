/**
 * \file
 *
 * \brief SPI basic driver.
 *
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
 *
 */

#include <spi_basic.h>
#include <atmel_start_pins.h>

typedef struct SPI_0_descriptor_s {
	spi_transfer_status_t  status;
	uint8_t *              data;
	uint8_t                size;
	spi_transfer_type_t    type;
	spi_transfer_done_cb_t cb;
} SPI_0_descriptor_t;

static SPI_0_descriptor_t SPI_0_desc;

void SPI_0_init()
{

	/* Enable SPI */
	PRR0 &= ~(1 << PRSPI);

	SPCR = 1 << SPE                     /* SPI module enable: enabled */
	       | 0 << DORD                  /* Data order: disabled */
	       | 1 << MSTR                  /* Master/Slave select: enabled */
	       | 0 << CPOL                  /* Clock polarity: disabled */
	       | 0 << CPHA                  /* Clock phase: disabled */
	       | (0 << SPR1) | (1 << SPR0); /* SPI Clock rate selection: fosc/64 */
	 //      | (0 << SPR1) | (0 << SPR0); /* SPI Clock rate selection: fosc/4 */

	SPSR = (0 << SPI2X); /* Disable double SPI speed */

	SPI_0_desc.status = SPI_FREE;
	SPI_0_desc.cb     = NULL;
}

void SPI_0_enable()
{
	SPCR |= (1 << SPE) | (1 << SPIE);	// SPIE added by Bob
}

void SPI_0_disable()
{
	SPCR &= ~(1 << SPE);
}

/*
  Callback function is typically used to:
  In master mode: Release SS after transfer as finished.
  In slave mode: Implement SPI protocol, setting up next transfer
                 after the previous one (i.e. data transfer phase
                 after control/command phase).
*/
void SPI_0_register_callback(spi_transfer_done_cb_t f)
{
	SPI_0_desc.cb = f;
}

ISR(SPI_STC_vect)
{
	/* SPI_0_desc.data points to array element
	   to write the received data to. The data to be transmitted
	   is in the next array element.
	*/
	uint8_t rdata = SPDR;
	uint8_t wdata = 0;
	if (SPI_0_desc.type != SPI_WRITE) {
		*SPI_0_desc.data = rdata;
	}

	SPI_0_desc.data++;

	if (SPI_0_desc.type != SPI_READ)
		wdata = *SPI_0_desc.data;

	SPI_0_desc.size--;
	// if more bytes to be transferred
	if (SPI_0_desc.size != 0) {
		// more data to send, send a byte
		SPDR = wdata;
	}

	// if last byte has been transferred, update status
	// and optionally call callback
	else {
		SPI_0_desc.status = SPI_DONE;
		if (SPI_0_desc.cb != NULL) {
			SPI_0_desc.cb();
		}
	}
}

bool SPI_0_status_free()
{
	return (SPI_0_desc.status == SPI_FREE);
}

bool SPI_0_status_idle()
{
	return (SPI_0_desc.status == SPI_IDLE);
}

bool SPI_0_status_busy()
{
	return (SPI_0_desc.status == SPI_BUSY);
}

bool SPI_0_status_done()
{
	return (SPI_0_desc.status == SPI_DONE);
}

uint8_t SPI_0_exchange_byte(uint8_t data)
{
	// Blocking wait for SPI free makes the function work
	// seamlessly also with IRQ drivers.
	while (SPI_0_desc.status == SPI_BUSY)
		;
	SPDR = data;
	while (!(SPSR & (1 << SPIF)))
		;
	return SPDR;
}

void SPI_0_exchange_block(void *block, uint8_t size)
{
	SPI_0_desc.data   = (uint8_t *)block;
	SPI_0_desc.size   = size;
	SPI_0_desc.type   = SPI_EXCHANGE;
	SPI_0_desc.status = SPI_BUSY;

	SPDR = *SPI_0_desc.data;
}

void SPI_0_write_block(void *block, uint8_t size)
{
	SPI_0_desc.data   = (uint8_t *)block;
	SPI_0_desc.size   = size;
	SPI_0_desc.type   = SPI_WRITE;
	SPI_0_desc.status = SPI_BUSY;

	SPDR = *SPI_0_desc.data;
}

void SPI_0_read_block(void *block, uint8_t size)
{
	SPI_0_desc.data   = (uint8_t *)block;
	SPI_0_desc.size   = size;
	SPI_0_desc.type   = SPI_READ;
	SPI_0_desc.status = SPI_BUSY;

	SPDR = 0;
}
