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

#include "pld-lcd.h"
#include "pld-uart.h"
#include "pld-i2c.h"

#define INA219 (0x40 << 1)

int main( void ) {

	char outs[20];

	uint8_t shunt_high, shunt_low;
	uint8_t bus_high, bus_low;
	uint8_t lcd_loop_count = 0;

	// Initialise the 16x2 LCD
	//
	lcd_init();

	// Serial port output
	//
	PORTB.DIR |= (1<< PIN0_bp);
	PORTB.OUT |= (1<< PIN0_bp); // serial output port, so push it high by default

	// Initialise the I2C SCL / SDA pair
	//
	i2c_init();


	uart_send_string("\r\nPLD-UCM4-T414\r\n");

	while (1) {
		uint8_t a;

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

		snprintf(outs,sizeof(outs),"%02x%02x%02x%02x\n", shunt_high, shunt_low, bus_high, bus_low );
		uart_send_string( outs );


		if (lcd_loop_count == 0) {
			lcd_loop_count = 5;


			uint16_t voltage;
			int16_t current_reg;
			int32_t current;
			uint32_t v, dv;

			voltage	= (bus_high << 8);
			voltage += bus_low;
			voltage = voltage >> 3;//	+ bus_low) >> 3;// *VOLTAGE_QUANTA; // 4mV per unit
										  //
										  //uint32_t current = (((shunt_high & 0x8F) << 8) + shunt_low) *20; // 0.05R shunt


			v = voltage *4;
			voltage = v;
			v = voltage /1000;
			dv = voltage -(v *1000);

			if (((v == 0) && (dv < 200)) || ((bus_low & 0x01)==1)) {
				snprintf(outs,17,"-               ");
				lcd_home();
				lcd_write_str(outs);
				lcd_gotoxy(0,1);
				lcd_write_str(outs);

			} else {
				uint32_t abs_current;

				snprintf(outs,17,"%2lu.%03luV", v, dv);
				lcd_gotoxy(1,0);
				lcd_write_str(outs);

				current_reg = (shunt_high << 8) + shunt_low; // compose the register
				current = current_reg ; // 0.05R shunt, so I = V/R, I = shunt voltage / 0.05 => I = shunt_voltage *20;
				current = current *20;
				current = current /100;
				if (current < 0) abs_current = -current;
				else abs_current = current;
				v = abs_current /1000;
				dv = abs_current -(v *1000);

				snprintf(outs,17,"%1lu.%03luA", v, dv);
				lcd_gotoxy(9,0);
				lcd_write_str(outs);

				if (current > 0) {
					lcd_gotoxy(0,0);
					lcd_write_data('>');
					lcd_gotoxy(15,0);
					lcd_write_data('>');
				} else {
					lcd_gotoxy(0,0);
					lcd_write_data('<');
					lcd_gotoxy(15,0);
					lcd_write_data('<');
				}
			}

		} // lcd_loop_count == 0;
		  //
		lcd_loop_count--;

		_delay_ms(50);
	} // while (1) big loop
	return 0;
}
