/**
 * \file
 *
 * \brief NVMCTRL basic driver example
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

#include <string.h>
#include <atmel_start.h>
#include <nvmctrl_basic_example.h>
#include <nvmctrl_basic.h>

// Some devices, e.g. tiny817, does not define SPM_PAGESIZE in its header file.
// For these devices, the rambuf variable is not used by the NVMCTL driver.
// Just give SPM_PAGESIZE a value here so that this example function compiles
// and executes correctly.
#ifndef SPM_PAGESIZE
#define SPM_PAGESIZE 1
#endif

static uint8_t                   rambuf[SPM_PAGESIZE];
static uint8_t                   wdata[] = {0, 1, 2, 3};
static uint8_t                   rdata[4];
static volatile nvmctrl_status_t status;
static volatile uint8_t          rb;

/*
NOTE:
Depending on the compiler and device used, the flash programming
routines may have to be placed in a separate segment. This segment is called:
* For GCC: .bootloader. Segment must be given a location in linker
           file or Toolchain->AVR/GNU Linker->Memory Settings in STUDIO.
* For IAR: BOOTLOADER_SEGMENT. Segment must be added to linker file.

Refer to driver documentation for more details.
*/

uint8_t FLASH_0_test_nvmctrl_basic(void)
{
	uint16_t i;

	//  Test EEPROM write
	FLASH_0_write_eeprom_byte(0, wdata[1]);
	rdata[1] = FLASH_0_read_eeprom_byte(0);
	if (rdata[1] != wdata[1])
		return 0; // Error

	FLASH_0_write_eeprom_block(0, wdata, 4);
	FLASH_0_read_eeprom_block(0, rdata, 4);
	if (memcmp(wdata, rdata, 4) != 0)
		return 0; // Error

	// Test flash write
	status = FLASH_0_write_flash_byte(2048 + 0, rambuf, 1);
	rb     = FLASH_0_read_flash_byte(2048 + 0);
	if (rb != 1)
		return 0; // Error
	status = FLASH_0_write_flash_byte(2048 + 1, rambuf, 2);
	rb     = FLASH_0_read_flash_byte(2048 + 1);
	if (rb != 2)
		return 0; // Error
	status = FLASH_0_write_flash_byte(2048 + 2, rambuf, 3);
	rb     = FLASH_0_read_flash_byte(2048 + 2);
	if (rb != 3)
		return 0; // Error

	for (i = 0; i < 2 * SPM_PAGESIZE; i++)
		// Init two pages in flash, starting at address 2048-ONE_PAGE.
		status = FLASH_0_write_flash_byte(2048 - SPM_PAGESIZE + i, rambuf, i);

	rb = FLASH_0_read_flash_byte(2048 - SPM_PAGESIZE);
	if (rb != 0)
		return 0; // Error
	rb = FLASH_0_read_flash_byte(2048 - 2);
	if (rb != SPM_PAGESIZE - 2)
		return 0; // Error
	rb = FLASH_0_read_flash_byte(2048 - 1);
	if (rb != SPM_PAGESIZE - 1)
		return 0; // Error
	rb = FLASH_0_read_flash_byte(2048);
	if (rb != SPM_PAGESIZE)
		return 0; // Error
	rb = FLASH_0_read_flash_byte(2048 + 1);
	if (rb != SPM_PAGESIZE + 1)
		return 0; // Error

	FLASH_0_write_flash_block(2048 - 2, wdata, 4, rambuf);

	rb = FLASH_0_read_flash_byte(2048 - SPM_PAGESIZE);
	if (rb != 0)
		return 0; // Error
	rb = FLASH_0_read_flash_byte(2048 - 2);
	if (rb != wdata[0])
		return 0; // Error
	rb = FLASH_0_read_flash_byte(2048 - 1);
	if (rb != wdata[1])
		return 0; // Error
	rb = FLASH_0_read_flash_byte(2048);
	if (rb != wdata[2])
		return 0; // Error
	rb = FLASH_0_read_flash_byte(2048 + 1);
	if (rb != wdata[3])
		return 0; // Error

	// If we get here, everything was OK
	return 1;
}
