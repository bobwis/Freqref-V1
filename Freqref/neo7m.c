/*
* neo7m.c
*
* Created: 13/09/2017 5:40:10 PM
*  Author: bob
*
* Defines originated from
* UBX GPS Library
* Created by Danila Loginov, July 23, 2016
* https://github.com/1oginov/UBX-GPS-Library

*/
#include <atmel_start.h>
#include "timeutils.h"
#include "usart_basic.h"
#include <stdio.h>


/**
* UBX GPS Library
* Created by Danila Loginov, July 2, 2016
* https://github.com/1oginov/UBX-GPS-Library
*
* Sketch, restoring the receiver's default configuration and configure it to get
* NAV-PVT messages with 100 ms frequency and 115200 baudrate. After the auto-
* configuration transmits the data from the receiver to the PC and vice versa.

*
* U-blox NEO-7M - Arduino Mega
* VCC - 5V
* RX - TX3
* TX - RX3
* GND - GND
*/

typedef uint8_t byte;

#define PC_SERIAL   Serial
#define PC_BAUDRATE 9600L
#define printgps  Serial3

// Default baudrate is determined by the receiver's manufacturer

#define GPS_DEFAULT_BAUDRATE    9600L

// Wanted buadrate can be 9600L (not changed after defaults) or 115200L (changed
// by the changeBaudrate function with prepared message) at the moment


#define GPS_WANTED_BAUDRATE     9600L

// Array of possible baudrates that can be used by receiver, sorted descending
// to prevent excess Serial flush/begin after restoring defaults. You can uncomment
// values that can be used by your receiver before the auto-configuration.
//


const long possibleBaudrates[] = {
	//921600L,
	//460800L,
	//230400L,
	115200L,
	//57600L,
	//38400L,
	//19200L,
	9600L,
	//4800L,
};

void printgps(unsigned char string[])
{
	int i = 0;
	uint8_t data;

	while((data = string[i++]))
	{
		USART_1_write(data);
	}
}


// Function, printing packet to the PC's serial in hexadecimal form
void printPacket(byte *packet, byte len) {
	char temp[3];

	for (byte i = 0; i < len; i++) {
		sprintf(temp, "%.2X", packet[i]);
		printf(temp);
		if (i != len - 1) {
			printf(" ");
		}
	}
	printf("\n\r");
}

// Function, sending specified packed to the GPS receiver
void sendPacket(byte *packet, byte len) {
	for (byte i = 0; i < len; i++) {
		USART_1_write(packet[i]);
	}

	printPacket(packet, len);
}

// Function, sending packet to the receiver to restore default configuration
void restoreDefaults() {
	// CFG-CFG packet
	byte packet[] = {
		0xB5, // sync char 1
		0x62, // sync char 2
		0x06, // class
		0x09, // id
		0x0D, // length
		0x00, // length
		0xFF, // payload
		0xFF, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0xFF, // payload
		0xFF, // payload
		0x00, // payload
		0x00, // payload
		0x17, // payload
		0x2F, // CK_A
		0xAE, // CK_B
	};

	sendPacket(packet, sizeof(packet));
}

// Function, sending set of packets to the receiver to disable NMEA messages
void disableNmea() {
	// Array of two bytes for CFG-MSG packets payload
	byte messages[][2] = {
		{0xF0, 0x0A},
		{0xF0, 0x09},
		{0xF0, 0x00},
		{0xF0, 0x01},
		{0xF0, 0x0D},
		{0xF0, 0x06},
		{0xF0, 0x02},
		{0xF0, 0x07},
		{0xF0, 0x03},
		{0xF0, 0x04},
		{0xF0, 0x0E},
		{0xF0, 0x0F},
		{0xF0, 0x05},
		{0xF0, 0x08},
		{0xF1, 0x00},
		{0xF1, 0x01},
		{0xF1, 0x03},
		{0xF1, 0x04},
		{0xF1, 0x05},
		{0xF1, 0x06},
	};

	// CFG-MSG packet buffer
	byte packet[] = {
		0xB5, // sync char 1
		0x62, // sync char 2
		0x06, // class
		0x01, // id
		0x03, // length
		0x00, // length
		0x00, // payload (first byte from messages array element)
		0x00, // payload (second byte from messages array element)
		0x00, // payload (not changed in the case)
		0x00, // CK_A
		0x00, // CK_B
	};
	byte packetSize = sizeof(packet);

	// Offset to the place where payload starts
	byte payloadOffset = 6;

	// Iterate over the messages array
	for (byte i = 0; i < sizeof(messages) / sizeof(*messages); i++) {
		// Copy two bytes of payload to the packet buffer
		for (byte j = 0; j < sizeof(*messages); j++) {
			packet[payloadOffset + j] = messages[i][j];
		}

		// Set checksum bytes to the null
		packet[packetSize - 2] = 0x00;
		packet[packetSize - 1] = 0x00;

		// Calculate checksum over the packet buffer excluding sync (first two)
		// and checksum chars (last two)
		for (byte j = 0; j < packetSize - 4; j++) {
			packet[packetSize - 2] += packet[2 + j];
			packet[packetSize - 1] += packet[packetSize - 2];
		}

		sendPacket(packet, packetSize);
	}
}

// Function, sending packet to the receiver to change baudrate to 115200
void changeBaudrate() {
	// CFG-PRT packet
	byte packet[] = {
		0xB5, // sync char 1
		0x62, // sync char 2
		0x06, // class
		0x00, // id
		0x14, // length
		0x00, // length
		0x01, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0xD0, // payload
		0x08, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0xC2, // payload
		0x01, // payload
		0x00, // payload
		0x07, // payload
		0x00, // payload
		0x03, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0x00, // payload
		0xC0, // CK_A
		0x7E, // CK_B
	};

	sendPacket(packet, sizeof(packet));
}

// Function, sending packet to the receiver to change frequency to 100 ms
void changeFrequency() {
	// CFG-RATE packet
	byte packet[] = {
		0xB5, // sync char 1
		0x62, // sync char 2
		0x06, // class
		0x08, // id
		0x06, // length
		0x00, // length
		0x64, // payload
		0x00, // payload
		0x01, // payload
		0x00, // payload
		0x01, // payload
		0x00, // payload
		0x7A, // CK_A
		0x12, // CK_B
	};

	sendPacket(packet, sizeof(packet));
}

// Function, sending packet to the receiver to disable unnecessary channels
void disableUnnecessaryChannels() {
	// CFG-GNSS packet
	byte packet[] = {
		0xB5, // sync char 1
		0x62, // sync char 2
		0x06, // class
		0x3E, // id
		0x24, // length
		0x00, // length

		0x00, 0x00, 0x16, 0x04, 0x00, 0x04, 0xFF, 0x00, // payload
		0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x03, 0x00, // payload
		0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x03, 0x00, // payload
		0x00, 0x00, 0x00, 0x01, 0x06, 0x08, 0xFF, 0x00, // payload
		0x00, 0x00, 0x00, 0x01, // payload

		0xA4, // CK_A
		0x25, // CK_B
	};

	sendPacket(packet, sizeof(packet));
}

// Function, sending packet to the receiver to enable NAV-PVT messages
void enableNavPvt() {
	// CFG-MSG packet
	byte packet[] = {
		0xB5, // sync char 1
		0x62, // sync char 2
		0x06, // class
		0x01, // id
		0x03, // length
		0x00, // length
		0x01, // payload
		0x07, // payload
		0x01, // payload
		0x13, // CK_A
		0x51, // CK_B
	};

	sendPacket(packet, sizeof(packet));
}




// If there is data from the receiver, read it and send to the PC or vice versa
void loop() {

	if (USART_1_is_rx_ready()) {
		USART_3_write(USART_1_read());
	}
	if (USART_3_is_rx_ready()) {
		USART_1_write(USART_3_read());

	}

}

void setupneo() {
	
	// Disabling NMEA messages by sending appropriate packets
	printf("Disabling NMEA messages...\n\r");
	disableNmea();

	// Switching receiver's serial to the wanted baudrate
	if (GPS_WANTED_BAUDRATE != GPS_DEFAULT_BAUDRATE) {
		printf("Switching receiver to the wanted baudrate which is ");
		printf("%ld",GPS_WANTED_BAUDRATE);
		printf("...\n\r");

		changeBaudrate();

		fastdelay_ms(100); // Little delay before flushing
	}

	// Increasing frequency to 100 ms
	printf("Changing receiving frequency to 100 ms...\n\r");
	changeFrequency();

	// Disabling unnecessary channels like SBAS or QZSS
	printf("Disabling unnecessary channels...\r\n");
	disableUnnecessaryChannels();

	// Enabling NAV-PVT messages
	printf("Enabling NAV-PVT messages...\n\r");
	enableNavPvt();

	printf("Auto-configuration is complete!\n\r");

	fastdelay_ms(100); // Little delay before flushing
	while(1)
		loop();
}
