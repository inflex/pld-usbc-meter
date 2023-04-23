/*****************************************************************
 *
 * This software was written by Paul L Daniels - pldaniels@pldaniels.com
 *
 *
 * NOTES
 *
 * ---------------------------------------------------------------------
 *  RELEASES
 *
 */

#define F_CPU 3333333UL 
#define F_OSC 3333333UL

/* User setting -> Software UART baudrate. */
#define UR_BAUDRATE	9600


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#include "pld-uart.h"
#include "pld-i2c.h"

#define INA219 (0x40 << 1)

char h2c( uint8_t h ) {
	if (h <= 9 && h >= 0) return '0'+h;
	if (h >= 10 && h <= 15) return 'a' +h -10;
}

void h2s( uint8_t h, char *s ) {
	*(s+1) = h2c(h & 0x0F);
	*(s) = h2c((h & 0xF0) >> 4);
	*(s+2) = '\0';
}

int main( void ) {

	char outs[20];

	uint8_t shunt_high, shunt_low;
	uint8_t bus_high, bus_low;
	uint8_t lcd_loop_count = 0;

	// Serial port output
	//
	PORTA.DIR |= (1<< PIN2_bp);
	PORTA.OUT |= (1<< PIN2_bp); // serial output port, so push it high by default

	// Initialise the I2C SCL / SDA pair
	//
	i2c_init();



	while (1) {
		uint8_t a;

		if (lcd_loop_count == 0) {
			uart_send_string("PLDUCM5M\n");
			lcd_loop_count = 10;
		}
		 
		lcd_loop_count--;

		i2c_start(INA219|I2C_WRITE);
		i2c_write_byte(0x01);
		i2c_start_rep( INA219|I2C_READ );
		shunt_high = i2c_read_byte(1);
		shunt_low = i2c_read_byte(0);
		i2c_stop();

		i2c_start(INA219|I2C_WRITE);
		i2c_write_byte(0x02);
		i2c_start_rep( INA219|I2C_READ );
		bus_high = i2c_read_byte(1);
		bus_low = i2c_read_byte(0);
		i2c_stop();

		h2s( shunt_high, outs );
		h2s( shunt_low, outs +2);
		h2s( bus_high, outs +4);
		h2s( bus_low, outs +6);
		outs[8] = '\n';
		outs[9] = '\0';

//		sprintf(outs,"%02x%02x%02x%02x\n", shunt_high, shunt_low, bus_high, bus_low );
		uart_send_string( outs );

		_delay_ms(10);
	} // while (1) big loop
	return 0;
}
