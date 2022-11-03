/*****************************************************************
 *
 * pld-uart.h : 9600-8n1 transmit service with bitbashing only
 *
 * This software was written by Paul L Daniels - pldaniels@gmail.com
 *
 *
 * NOTES
 *
 *
 * This is an incredibly simple bitbashed serial output / tx-only 
 * function;  useful if you need some slower output data being
 * blindly shoved out a pin.
 *
 * This was written for the ATTiny 1-series (T212, 414 etc)
 *  standard chip, no different fuses set
 *
 * It's not meant to be optimal
 * It's not meant to be efficient
 * It's not meant to be 100% correct
 * It is just meant to work in a very simple way
 *
 *
 * We're using 100us delays which should give us 9600bps 8n1
 *
 *		void uart_serial_send_string( const char *s )
 *
 * ---------------------------------------------------------------------
 *  RELEASES
 *
 */


// If we've not already defined the CPU/OSC speeds
// we will need to here in order for the _delay_ms()
// routines to work
//
//
#ifndef F_CPU
#define F_CPU 3333333UL 
#endif

#ifndef F_OSC
#define F_OSC 3333333UL
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>


// Set these 3 defines (SET_0, SET_1, DELAY)
// to what ever you need to in order to perform
// the function on your specific uC
//
// For this example, we're using PA0 (PORTA, PIN0) on the T212
//
// **NOTE** Be sure to set the port as an OUTPUT in your software
//
//
// Uncomment these #define's and set them to suit your system
#define SET_0 (PORTB.OUT &= ~(1<<PIN0_bp))
#define SET_1 (PORTB.OUT |= (1<<PIN0_bp))
#define DELAY _delay_us(100)

void uart_tx_byte( uint8_t b ) {
	uint8_t bc = 8; // 8 bits to shove out

	// Start bit
	// set the output to zero / low / 0
	//
	SET_0;
	DELAY;

	// Send off the 8 bits of data...
	//
	while (bc--) {

		// If we've got a 1, send a 1
		//
		if ((b & 0x01) == 1) { 
			SET_1;
		}

		// If we've got a 0, send a 0
		//
		if ((b & 0x01) == 0) {
			SET_0;
		}

		// Move to the next bit
		//
		b = b >> 1;

		// Delay ~100us
		//
		DELAY;
	}

	// Set the stop bit
	//
	SET_1;
	DELAY;

	// We're all done.
} // tx_byte()


// Send a nul/zero/'\0' terminated C 
// string through the serial port
//
void uart_send_string( const char *s ) {
	while (*s) {
		uart_tx_byte(*s);
		s++;
	}
}

//---------------END pld-uart.h---------------//
