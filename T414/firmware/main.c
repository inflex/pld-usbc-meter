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
#define LCD (0x3E <<1)


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




void lcd_cmd80(uint8_t db)
{
	i2c_write_byte(0x80);
	_delay_ms(1);
	i2c_write_byte(db);
}

void lcd_cmd0(uint8_t db)
{
	i2c_write_byte(0x00);
	i2c_write_byte(db);
}

void lcd_cmd(uint8_t db)
{
	i2c_start_wait(LCD +I2C_WRITE);
	_delay_ms(1);
	i2c_write_byte(0x80);
	_delay_ms(1);
	i2c_write_byte(db);
	i2c_stop();
	_delay_ms(1);
}

void lcd_data(uint8_t db)
{
	i2c_start_wait(LCD+I2C_WRITE);
	i2c_write_byte(0x40);
	i2c_write_byte(db);
	i2c_stop();
	_delay_ms(1);
}

void lcd_puts(uint8_t *s)
{
	while(*s){
		lcd_data(*s++);
	}
}



void lcd_init(void) {	

#define LCD_CONTRAST 0b110000 
	i2c_start(LCD+I2C_WRITE);
	lcd_cmd80(0x38);
	lcd_cmd80(0x39);
	lcd_cmd80(0x1c);
	lcd_cmd80(0x76); // lower 4 bits of contrast, ranges between 76~7a seem ok
	lcd_cmd80(0x56); // upper contrast, don't meddle with this one
	lcd_cmd80(0x6c);
	lcd_cmd80(0x0c);
	lcd_cmd80(0x01);
	//	lcd_cmd80(0x06);
	//	lcd_cmd80(0x02);
	i2c_stop();
}



int main( void ) {

	char outs[20];

	uint8_t shunt_high, shunt_low;
	uint8_t bus_high, bus_low;
	uint8_t lcd_loop_count = 0;




	// Serial port output
	//
	PORTB.DIR |= (1<< PIN0_bp);
	PORTB.OUT |= (1<< PIN0_bp); // serial output port, so push it high by default

	// Initialise the I2C SCL / SDA pair
	//
	i2c_init();



	_delay_ms(100); // we have to wait for the LCD to get ready, usually 40ms.
						 //
	lcd_init();
	_delay_ms(40);
	lcd_init();
	_delay_ms(40);

	lcd_puts((uint8_t *)"PLD-USBC");
	lcd_cmd(0xC0);	// ADDR=0x40
	lcd_puts((uint8_t *)"Meter");
	_delay_ms(2000);			 

	lcd_cmd(0x01); // Clear screen


	serial_send_string("\r\nPLD-UCM4-T414\r\n");

	while (1) {
		uint8_t a;


		/*
		 * Check to see if you can read the default
		 * settings register, which should be 0x399F
		 *
		 *
		 i2c_start(INA219|I2C_WRITE);
		 a = i2c_write_byte(0x00);
		 i2c_start_rep( INA219|I2C_READ );
		 shunt_high = i2c_read_byte(1);
		 shunt_low = i2c_read_byte(0);
		 i2c_stop();

		 snprintf(outs,sizeof(outs),"ack = %d id: %02x %02x ", a, shunt_high, shunt_low );
		 serial_send_string( outs );
		 */

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

		} // lcd_loop_count == 0;
		  //
		lcd_loop_count--;

		_delay_ms(50);
	} // while (1) big loop
	return 0;
}
