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


#define INA219 (0x40 << 1)

#define LCD_D4_HIGH (PORTA.OUTSET = (1<<PIN4_bp))
#define LCD_D5_HIGH (PORTA.OUTSET = (1<<PIN5_bp))
#define LCD_D6_HIGH (PORTA.OUTSET = (1<<PIN6_bp))
#define LCD_D7_HIGH (PORTA.OUTSET = (1<<PIN7_bp))

#define LCD_D4_LOW (PORTA.OUTCLR = (1<<PIN4_bp))
#define LCD_D5_LOW (PORTA.OUTCLR = (1<<PIN5_bp))
#define LCD_D6_LOW (PORTA.OUTCLR = (1<<PIN6_bp))
#define LCD_D7_LOW (PORTA.OUTCLR = (1<<PIN7_bp))

#define LCD_E_HIGH (PORTB.OUTSET = (1<<PIN3_bp))
#define LCD_RW_HIGH (PORTB.OUTSET = (1<<PIN2_bp))
#define LCD_RS_HIGH (PORTB.OUTSET = (1<<PIN1_bp))

#define LCD_E_LOW (PORTB.OUTCLR = (1<<PIN3_bp))
#define LCD_RW_LOW (PORTB.OUTCLR = (1<<PIN2_bp))
#define LCD_RS_LOW (PORTB.OUTCLR = (1<<PIN1_bp))

#define LCD_BUSY_FLAG (PORTA.IN & (1<<PIN7_bp))

#define LCD_CLK_DELAY (_delay_us(100))
#define LCD_E_DELAY (_delay_us(1))
#define LCD_E_PULSE (LCD_E_HIGH, LCD_E_DELAY, LCD_E_LOW)

void tx_byte( uint8_t b ) {
	uint8_t bc = 8; // 8 bits to shove out
	PORTB.OUT &= ~(1<< PIN0_bp); // signify start of data
	_delay_us(100);
	while (bc--) {
		if ((b & 0x01) == 1) { 
			PORTB.OUT |= (1<< PIN0_bp);
		}
		if ((b & 0x01) == 0) {
			PORTB.OUT &= ~(1<< PIN0_bp);
		}
		b = b >> 1;
		_delay_us(100);
	}
	PORTB.OUT |= (1<< PIN0_bp);
	_delay_us(100);
}

void serial_send_string( const char *s ) {
	while (*s) {
		tx_byte(*s);
		_delay_ms(1);
		s++;
	}
}

#define I2C_READ 1
#define I2C_WRITE 0

uint8_t read_SDA (void) {
	return (PORTA.IN & (1<<PIN2_bp));
}
void sda_high(void) {
	PORTA.DIRCLR = (1<<PIN2_bp);
	PORTA.OUTCLR = (1<<PIN2_bp);
}
void sda_low(void) {
	PORTA.DIRSET = (1 << PIN2_bp);
}

uint8_t read_SCL(void) {
	return (PORTA.IN & (1<<PIN3_bp));
}
void scl_high(void) {
	PORTA.DIRCLR = (1<<PIN3_bp);
	PORTA.OUTCLR = (1<<PIN3_bp);
}
void scl_low(void) {
	PORTA.DIRSET = (1 << PIN3_bp);
}


#define Q_DELAY _delay_loop_2(3)
#define H_DELAY _delay_loop_2(5)


void i2c_init()
{
	sda_high();	
	scl_high();	

	PORTA.OUT &= ~(1<<PIN2_bp);
	PORTA.OUT &= ~(1<<PIN3_bp);


}
void i2c_wait_ack(void) {
	uint8_t ack = 1;
	sda_high();
	scl_high();
	H_DELAY;
	while ( ack == 1) {
		ack = read_SDA();
	}
	scl_low();
}

uint8_t i2c_write_byte(uint8_t data)
{

	uint8_t i;

	for(i=0;i<8;i++) {
		scl_low();

		if(data & 0x80) {
			sda_high();
		}else {
			sda_low();	
		}
		H_DELAY;
		scl_high();
		H_DELAY;
		data=data<<1;
	}

	// Data has been sent, now listen for the ACK
	//
	scl_low();
	H_DELAY;

	scl_high();		
	//	H_DELAY; // optional
	while (read_SCL()==0);

	uint8_t ack;
	ack = read_SDA();

	H_DELAY;

	return ack;

}




uint8_t i2c_write_byte_ackwait(uint8_t data)
{

	uint8_t i;

	for(i=0;i<8;i++) {
		scl_low();

		if(data & 0x80) {
			sda_high();
		}else {
			sda_low();	
		}
		H_DELAY;
		scl_high();
		H_DELAY;
		data=data<<1;
	}

	// Data has been sent, now listen for the ACK
	//
	scl_low();
	H_DELAY;

	scl_high();		
	//	H_DELAY; // optional
	while (read_SCL()==0);

	i2c_wait_ack();

	H_DELAY;

	return 0;

}




void i2c_stop()
{
	scl_low();
	sda_low();
	H_DELAY;
	scl_high();
	Q_DELAY;
	sda_high();
	H_DELAY;
}


uint8_t i2c_read_byte(uint8_t ack)
{
	uint8_t data=0x00;
	uint8_t i;

	for(i=0;i<8;i++)
	{

		scl_low();
		sda_high(); // Without this, the uC sent ACK doesn't seem to work
		H_DELAY;
		scl_high();
		H_DELAY;

		//		while(read_SCL()==0);

		if (read_SDA()) data |= 1;
		if (i < 7) data <<= 1;

	}

	scl_low();
	if (ack == 1) sda_low();
	else sda_high();
	H_DELAY;
	scl_high();

	while ( read_SCL() == 0 ); // wait for slave to finish if it's stretching

	H_DELAY;

	return data;

}


uint8_t i2c_start( uint8_t addr ) {
	sda_low();
	Q_DELAY;
	return i2c_write_byte(addr);
}

uint8_t i2c_start_rep( uint8_t addr ) {
	scl_low();
	sda_high();
	H_DELAY;
	scl_high();
	Q_DELAY;
	sda_low();
	Q_DELAY;

	return i2c_write_byte(addr);
}


uint8_t i2c_start_wait( uint8_t addr ) {
	while ( 1 ) {
		sda_low();
		H_DELAY;
		if (i2c_write_byte(addr) == 0) break;
		i2c_stop();
	}
	return 0;
}




int lcd_write_byte( uint8_t d, int mode ) {


	if ( mode == 0 ) {
		LCD_RS_LOW;
		LCD_RW_LOW;
	} else {
		LCD_RS_HIGH;
		LCD_RW_LOW;
	}

	if (d & 0x80) LCD_D7_HIGH; else LCD_D7_LOW;
	if (d & 0x40) LCD_D6_HIGH; else LCD_D6_LOW;
	if (d & 0x20) LCD_D5_HIGH; else LCD_D5_LOW;
	if (d & 0x10) LCD_D4_HIGH; else LCD_D4_LOW;
	LCD_E_HIGH;
	LCD_E_DELAY;
	LCD_E_LOW;


	if (d & 0x08) LCD_D7_HIGH; else LCD_D7_LOW;
	if (d & 0x04) LCD_D6_HIGH; else LCD_D6_LOW;
	if (d & 0x02) LCD_D5_HIGH; else LCD_D5_LOW;
	if (d & 0x01) LCD_D4_HIGH; else LCD_D4_LOW;
	LCD_E_HIGH;
	LCD_E_DELAY;
	LCD_E_LOW;

	
	LCD_D7_HIGH;
	LCD_D6_HIGH;
	LCD_D5_HIGH;
	LCD_D4_HIGH;

	if (mode != 0) {
		LCD_RS_LOW;
	}
}

int lcd_read( uint8_t rs ) {
	uint8_t data = 0;

	if (rs == 1) LCD_RS_HIGH;
	else LCD_RS_LOW;

	LCD_RW_HIGH;
	PORTA.DIRCLR = (1<<PIN4_bp)|(1<<PIN5_bp)|(1<<PIN6_bp)|(1<<PIN7_bp);

	LCD_E_HIGH;
	LCD_CLK_DELAY;
	if (PORTA.IN & (1<<PIN7_bp)) data |= 0x80;
	if (PORTA.IN & (1<<PIN6_bp)) data |= 0x40;
	if (PORTA.IN & (1<<PIN5_bp)) data |= 0x20;
	if (PORTA.IN & (1<<PIN4_bp)) data |= 0x10;
	LCD_E_LOW;
	LCD_E_DELAY;


	LCD_E_HIGH;
	LCD_E_DELAY;
	if (PORTA.IN & (1<<PIN7_bp)) data |= 0x08;
	if (PORTA.IN & (1<<PIN6_bp)) data |= 0x04;
	if (PORTA.IN & (1<<PIN5_bp)) data |= 0x02;
	if (PORTA.IN & (1<<PIN4_bp)) data |= 0x01;
	LCD_E_LOW;

	PORTA.DIRSET = (1<<PIN4_bp)|(1<<PIN5_bp)|(1<<PIN6_bp)|(1<<PIN7_bp);

	LCD_RW_LOW;

	return data;

}

int lcd_wait_if_busy(void) { 

	uint8_t d;
	while (1) {
		d = lcd_read(0);
		if ((d & 0x80) == 0) break;
	}
	LCD_CLK_DELAY;
	d = lcd_read(0);

	return d; // address counter is returned
}

void lcd_write_cmd( uint8_t cmd ) {
	lcd_wait_if_busy();
	lcd_write_byte( cmd, 0 );
}

void lcd_write_data( uint8_t data ) {
	lcd_wait_if_busy();
	lcd_write_byte( data, 1 );
}

void lcd_write_str( char *s ) {
	while (*s) {
		lcd_write_data(*s);
		s++;
	}
}

void lcd_gotoline( uint8_t line ) {
	if ( line == 0 ) {
		lcd_write_cmd( 0x80 + 0x00 );
	} else if (line == 1) {
		lcd_write_cmd( 0x80 + 0x40 );
	}
}


void lcd_init( void ) {
	// Set our data pins D4:7 to outputs
	//
	PORTA.DIR |= (1<<PIN4_bp)|(1<<PIN5_bp)|(1<<PIN6_bp)|(1<<PIN7_bp);

	// Set our control lines (E, RS, RW) to outputs
	//
	PORTB.DIR |= (1<<PIN3_bp)|(1<<PIN2_bp)|(1<<PIN1_bp);

	// Prep the control lines
	//
	LCD_RS_LOW;
	LCD_RW_LOW;
	LCD_E_LOW;

	// Set the initial function request; we don't use
	// the _write_cmd() call yet because it wants to
	// check for the busy flag
	//
	// This initial 0x3 value for FunctionSet will push
	// the LCD in to using 4-bit/nibble mode
	// 
	LCD_D7_LOW;
	LCD_D6_LOW;
	LCD_D5_HIGH;
	LCD_D4_HIGH;
	
	LCD_E_PULSE;
	_delay_ms(20);

	// Repeat
	//
	LCD_E_PULSE;
	_delay_ms(2);

	// Repeat
	//
	LCD_E_PULSE;
	_delay_ms(2);

	// Now drop the 0th bit on the high nibble for 'DL' 
	// to indicate that we want to move to 4-bit mode
	//
	LCD_D4_LOW;
	LCD_E_PULSE;
	_delay_ms(40);


	// Now we can start issueing commands with the busy flag
	// response being available
	//
	lcd_write_cmd(0x0C); // display on, cursor off, blink off
	_delay_us(100);

	lcd_write_cmd(0x06); // new data increments position in DDRAM
	_delay_ms(2);

	lcd_write_cmd(0x01); // clear LCD and set home
	_delay_ms(2);

}

void lcd_clrscr( void ) { 
	lcd_write_cmd( 0x01 ); // moves address back to top-left of LCD and sets all data to spaces
}

void lcd_home( void ) {
	lcd_write_cmd( 0x02 ); // moves address back to top-left only
}

int main( void ) {

	char outs[20];

	uint8_t shunt_high, shunt_low;
	uint8_t bus_high, bus_low;
	uint8_t lcd_loop_count = 0;

	lcd_init();

	// Serial port output
	//
	PORTB.DIR |= (1<< PIN0_bp);
	PORTB.OUT |= (1<< PIN0_bp); // serial output port, so push it high by default

	// Initialise the I2C SCL / SDA pair
	//
	i2c_init();


	serial_send_string("\r\nPLD-UCM4-T414\r\n");

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
		serial_send_string( outs );


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
				lcd_gotoline(1);
				lcd_write_str(outs);

			} else {

				snprintf(outs,17,"%2lu.%03luV ", v, dv);
				lcd_home();
				lcd_write_str(outs);
				_delay_ms(3);


				uint32_t abs_current;

				current_reg = (shunt_high << 8) + shunt_low; // compose the register
				current = current_reg ; // 0.05R shunt, so I = V/R, I = shunt voltage / 0.05 => I = shunt_voltage *20;
				current = current *20;
				current = current /100;
				if (current < 0) abs_current = -current;
				else abs_current = current;
				v = abs_current /1000;
				dv = abs_current -(v *1000);
				if (current > 0) snprintf(outs,17,">%1lu.%03luA        >", v, dv);
				else snprintf(outs,17,"<%1lu.%03luA        <", v, dv);

				outs[16] = 0;
				lcd_gotoline(1);
				lcd_write_str(outs);
			}

		} // lcd_loop_count == 0;
		  //
		lcd_loop_count--;

		_delay_ms(50);
	} // while (1) big loop
	return 0;
}
