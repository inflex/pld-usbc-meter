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

#define F_CPU 8000000UL
#define F_OSC 8000000UL

/* User setting -> Output pin the software UART */
#define UR_TX_PORT	PORTB
#define UR_TX_DDR		DDRB
#define UR_TX_BIT		PB3
#define UR_RX_BIT		PB4

/* User setting -> Software UART baudrate. */
#define UR_BAUDRATE	9600


#include "i2cmaster.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>


#define INA219 (0x40 << 1)
#define LCD (0x3E <<1)

void serial_send_byte(uint8_t c)
{
#define UR_TX_NUM_DELAY_CYCLES	((F_CPU/UR_BAUDRATE-16)/4+1)
#define UR_TX_NUM_ADD_NOP		((F_CPU/UR_BAUDRATE-16)%4)
	uint8_t sreg;
	uint16_t tmp;
	uint8_t numiter = 10;

	sreg = SREG;
	cli();

	asm volatile (
			/* put the START bit */
			"in %A0, %3"		"\n\t"	/* 1 */
			"cbr %A0, %4"		"\n\t"	/* 1 */
			"out %3, %A0"		"\n\t"	/* 1 */
			/* compensate for the delay induced by the loop for the
			 * other bits */
			"nop"			"\n\t"	/* 1 */
			"nop"			"\n\t"	/* 1 */
			"nop"			"\n\t"	/* 1 */
			"nop"			"\n\t"	/* 1 */
			"nop"			"\n\t"	/* 1 */

			/* delay */
			"1:" "ldi %A0, lo8(%5)"	"\n\t"	/* 1 */
			"ldi %B0, hi8(%5)"	"\n\t"	/* 1 */
			"2:" "sbiw %A0, 1"		"\n\t"	/* 2 */
			"brne 2b"		"\n\t"	/* 1 if EQ, 2 if NEQ */
#if UR_TX_NUM_ADD_NOP > 0
			"nop"			"\n\t"	/* 1 */
#if UR_TX_NUM_ADD_NOP > 1
			"nop"			"\n\t"	/* 1 */
#if UR_TX_NUM_ADD_NOP > 2
			"nop"			"\n\t"	/* 1 */
#endif
#endif
#endif
			/* put data or stop bit */
			"in %A0, %3"		"\n\t"	/* 1 */
			"sbrc %1, 0"		"\n\t"	/* 1 if false,2 otherwise */
			"sbr %A0, %4"		"\n\t"	/* 1 */
			"sbrs %1, 0"		"\n\t"	/* 1 if false,2 otherwise */
			"cbr %A0, %4"		"\n\t"	/* 1 */
			"out %3, %A0"		"\n\t"	/* 1 */

			/* shift data, putting a stop bit at the empty location */
			"sec"			"\n\t"	/* 1 */
			"ror %1"		"\n\t"	/* 1 */

			/* loop 10 times */
			"dec %2"		"\n\t"	/* 1 */
			"brne 1b"		"\n\t"	/* 1 if EQ, 2 if NEQ */
			: "=&w" (tmp),			/* scratch register */
		"=r" (c),			/* we modify the data byte */
		"=r" (numiter)		/* we modify number of iter.*/
			: "I" (_SFR_IO_ADDR(UR_TX_PORT)),
			"M" (1<<UR_TX_BIT),
			"i" (UR_TX_NUM_DELAY_CYCLES),
			"1" (c),			/* data */
			"2" (numiter)
			       );
	SREG = sreg;
}
#undef UR_TX_NUM_DELAY_CYCLES
#undef UR_TX_NUM_ADD_NOP



void serial_send_string( const char *s ) {
	while (*s) {
		serial_send_byte(*s);
		_delay_ms(3);
		s++;
	}
}

void lcd_cmd(uint8_t db)
{
	i2c_start_wait(LCD +I2C_WRITE);
	i2c_write(0x00);
	i2c_write(db);
	i2c_stop();
	//	i2cSend(0b01111100, 0b00000000, 1, &db);
}

/*-----------------------------------------------------------------\
  Date Code:	: 20111012-205544
  Function Name	: i2c_data
  Returns Type	: void
  ----Parameter List
  1. uint8_t db,  
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
void lcd_data(uint8_t db)
{
	i2c_start_wait(LCD +I2C_WRITE);
	i2c_write(0x40);
	i2c_write(db);
	i2c_stop();
	//	i2cSend(0b01111100, 0b01000000, 1, &db);
}

/*-----------------------------------------------------------------\
  Date Code:	: 20111012-204922
  Function Name	: i2c_puts
  Returns Type	: void
  ----Parameter List
  1. uint8_t *s, 
  ------------------
  Exit Codes	: 
  Side Effects	: 
  --------------------------------------------------------------------
Comments:

--------------------------------------------------------------------
Changes:

\------------------------------------------------------------------*/
void lcd_puts(uint8_t *s)
{
	while(*s){
		lcd_data(*s++);
	}
}



int main( void ) {

	char outs[20];

	clock_prescale_set(clock_div_1);

	UR_TX_DDR |= (1<<UR_TX_BIT);


	i2c_init();

	/** 3.0V contrast **/
#define CONTRAST 0b110000 

	uint8_t loop = 2;
	while (loop-- > 0) {
		_delay_ms(300);
	DDRB |= (1<<PB0);
	PORTB |= (1<<PB0);


	lcd_cmd(0b00111000); // function set
	lcd_cmd(0b00111001); // function set
	lcd_cmd(0b00010100); // interval osc
	lcd_cmd(0b01011100 | ((CONTRAST >> 4) & 0x3)); // contast High/icon/power
	lcd_cmd(0b01101100); // follower control
	_delay_ms(300);


	lcd_cmd(0b00111000); // function set
	lcd_cmd(0b00001100); // Display On

	lcd_cmd(0b00000001); // Clear screen
			     //
//	lcd_puts((uint8_t *)"PLD-USBC");
//	lcd_cmd(0b11000000);	// ADDR=0x40
//	lcd_puts((uint8_t *)"Meter");
//	_delay_ms(2000);			 
	}
//	
//	while(1);

//	_delay_ms(300);

#define VOLTAGE_QUANTA 4

//	while (1) {
//		serial_send_string("U");
//		_delay_ms(100);
//	}

	uint8_t lcd_loop_count = 0;

	while (1) {

		uint8_t shunt_high, shunt_low;
		uint8_t bus_high, bus_low;




		i2c_start_wait(INA219+I2C_WRITE);
		i2c_write(0x01);
		i2c_rep_start(INA219+I2C_READ);
		shunt_high = i2c_readAck();
		shunt_low = i2c_readNak();
		i2c_stop();

		i2c_start_wait(INA219+I2C_WRITE);
		i2c_write(0x02);
		i2c_rep_start(INA219+I2C_READ);
		bus_high = i2c_readAck();
		bus_low = i2c_readNak();
		i2c_stop();

		snprintf(outs,sizeof(outs),"%02x%02x", shunt_high, shunt_low );
		serial_send_string( outs );
		snprintf(outs,sizeof(outs),"%02x%02x", bus_high, bus_low);
		serial_send_string( outs );
		serial_send_string( "\n" );

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
			snprintf(outs,9,"%2lu.%03luV ", v, dv);

			if (((v == 0) && (dv < 200)) || ((bus_low & 0x01)==1)) {
				snprintf(outs,9,"-       ");
				lcd_cmd(0b00000010); // Go home
				lcd_puts((uint8_t *)outs);
				lcd_cmd(0b11000000);	// ADDR=0x40
				lcd_puts((uint8_t *)outs);

			} else {

				lcd_cmd(0b00000010); // Go home
				_delay_ms(3);
				outs[8] = 0;
				lcd_puts((uint8_t *)outs);

				uint32_t abs_current;

				lcd_cmd(0b11000000);	// ADDR=0x40
				current_reg = (shunt_high << 8) + shunt_low; // compose the register
				current = current_reg ; // 0.05R shunt, so I = V/R, I = shunt voltage / 0.05 => I = shunt_voltage *20;
				current = current *20;
				current = current /100;
				if (current < 0) abs_current = -current;
				else abs_current = current;
				v = abs_current /1000;
				dv = abs_current -(v *1000);
				if (current < 0) snprintf(outs,9,">%1lu.%03luA>", v, dv);
				else snprintf(outs,9,"<%1lu.%03luA<", v, dv);

				outs[8] = 0;
				lcd_puts((uint8_t *)outs);
			}
		}
		lcd_loop_count--;

		_delay_ms(50);
	}
	return 0;
}
