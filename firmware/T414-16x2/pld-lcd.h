//---------------- pld-lcd.h -------------------------------
//
// Simple header-included code for driving ST7660 style 
// 16x2 LCD in 4-bit nibble mode for 1-series Attiny chips
// (this was done on an Attiny414)
//
// This code is NOT optimal for space or speed but it does
// serve as a functional option to get started from
//
// 
// Pins/ports can be set to basically any combination you
// like on the chip, so it lets you get away with adding 
// the display last and using up and spare pins even if 
// they're spread all over the chip
//
// Example code:
//
// int main(void) {
//
//		lcd_init();
//		lcd_write_str("Hello");
//		while(1);
//	}
//
//




// Define the PORT and PIN for each connection to the LCD
//
// You will need D4..D7, and E, RS and RW
//
//
#define LCD_D4_PIN PIN4_bp
#define LCD_D5_PIN PIN5_bp
#define LCD_D6_PIN PIN6_bp
#define LCD_D7_PIN PIN7_bp

#define LCD_D4_PORT PORTA
#define LCD_D5_PORT PORTA
#define LCD_D6_PORT PORTA
#define LCD_D7_PORT PORTA

#define LCD_E_PIN PIN3_bp
#define LCD_RW_PIN PIN2_bp
#define LCD_RS_PIN PIN1_bp

#define LCD_E_PORT PORTB
#define LCD_RS_PORT PORTB
#define LCD_RW_PORT PORTB
//
//
//-----------END of User defined section





//-----------You shouldn't have to change anything from here
//
//
#define LCD_D4_OUTPUT (LCD_D4_PORT.DIRSET = (1<<LCD_D4_PIN))
#define LCD_D5_OUTPUT (LCD_D5_PORT.DIRSET = (1<<LCD_D5_PIN))
#define LCD_D6_OUTPUT (LCD_D6_PORT.DIRSET = (1<<LCD_D6_PIN))
#define LCD_D7_OUTPUT (LCD_D7_PORT.DIRSET = (1<<LCD_D7_PIN))

#define LCD_D4_INPUT (LCD_D4_PORT.DIRCLR = (1<<LCD_D4_PIN))
#define LCD_D5_INPUT (LCD_D5_PORT.DIRCLR = (1<<LCD_D5_PIN))
#define LCD_D6_INPUT (LCD_D6_PORT.DIRCLR = (1<<LCD_D6_PIN))
#define LCD_D7_INPUT (LCD_D7_PORT.DIRCLR = (1<<LCD_D7_PIN))

#define LCD_D4_HIGH (LCD_D4_PORT.OUTSET = (1<<LCD_D4_PIN))
#define LCD_D5_HIGH (LCD_D5_PORT.OUTSET = (1<<LCD_D5_PIN))
#define LCD_D6_HIGH (LCD_D6_PORT.OUTSET = (1<<LCD_D6_PIN))
#define LCD_D7_HIGH (LCD_D7_PORT.OUTSET = (1<<LCD_D7_PIN))

#define LCD_D4_LOW (LCD_D4_PORT.OUTCLR = (1<<LCD_D4_PIN))
#define LCD_D5_LOW (LCD_D5_PORT.OUTCLR = (1<<LCD_D5_PIN))
#define LCD_D6_LOW (LCD_D6_PORT.OUTCLR = (1<<LCD_D6_PIN))
#define LCD_D7_LOW (LCD_D7_PORT.OUTCLR = (1<<LCD_D7_PIN))

#define LCD_D4_READ (LCD_D4_PORT.IN & (1<<LCD_D4_PIN))
#define LCD_D5_READ (LCD_D5_PORT.IN & (1<<LCD_D5_PIN))
#define LCD_D6_READ (LCD_D6_PORT.IN & (1<<LCD_D6_PIN))
#define LCD_D7_READ (LCD_D7_PORT.IN & (1<<LCD_D7_PIN))



#define LCD_E_HIGH (LCD_E_PORT.OUTSET = (1<<LCD_E_PIN))
#define LCD_RW_HIGH (LCD_RW_PORT.OUTSET = (1<<LCD_RW_PIN))
#define LCD_RS_HIGH (LCD_RS_PORT.OUTSET = (1<<LCD_RS_PIN))

#define LCD_E_LOW (LCD_E_PORT.OUTCLR = (1<<LCD_E_PIN))
#define LCD_RW_LOW (LCD_RW_PORT.OUTCLR = (1<<LCD_RW_PIN))
#define LCD_RS_LOW (LCD_RS_PORT.OUTCLR = (1<<LCD_RS_PIN))

#define LCD_E_OUTPUT (LCD_E_PORT.DIRSET = (1<<LCD_E_PIN))
#define LCD_RW_OUTPUT (LCD_RW_PORT.DIRSET = (1<<LCD_RW_PIN))
#define LCD_RS_OUTPUT (LCD_RS_PORT.DIRSET = (1<<LCD_RS_PIN))

#define LCD_BUSY_FLAG 0x80 

#define LCD_CLK_DELAY (_delay_us(100))
#define LCD_E_DELAY (_delay_us(1))
#define LCD_E_PULSE (LCD_E_HIGH, LCD_E_DELAY, LCD_E_LOW)



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
	LCD_D4_INPUT;
	LCD_D5_INPUT;
	LCD_D6_INPUT;
	LCD_D7_INPUT;

	LCD_E_HIGH;
	LCD_CLK_DELAY;
	if (LCD_D7_READ) data |= 0x80;
	if (LCD_D6_READ) data |= 0x40;
	if (LCD_D5_READ) data |= 0x20;
	if (LCD_D4_READ) data |= 0x10;
	LCD_E_LOW;
	LCD_E_DELAY;

	LCD_E_HIGH;
	LCD_E_DELAY;
	if (LCD_D7_READ) data |= 0x08;
	if (LCD_D6_READ) data |= 0x04;
	if (LCD_D5_READ) data |= 0x02;
	if (LCD_D4_READ) data |= 0x01;
	LCD_E_LOW;

	LCD_D4_OUTPUT;
	LCD_D5_OUTPUT;
	LCD_D6_OUTPUT;
	LCD_D7_OUTPUT;

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



void lcd_gotoxy( uint8_t col, uint8_t line ) {
	lcd_write_cmd( 0x80 +(line *0x40) +col );
}



void lcd_clrscr( void ) { 
	lcd_write_cmd( 0x01 ); // moves address back to top-left of LCD and sets all data to spaces
}

void lcd_home( void ) {
	lcd_write_cmd( 0x02 ); // moves address back to top-left only
}


void lcd_init( void ) {
	// Set our data pins D4:7 to outputs
	//
	LCD_D4_OUTPUT;
	LCD_D5_OUTPUT;
	LCD_D6_OUTPUT;
	LCD_D7_OUTPUT;

	// Set our control lines (E, RS, RW) to outputs
	//
	LCD_E_OUTPUT;
	LCD_RW_OUTPUT;
	LCD_RS_OUTPUT;

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

	// Repeat... these displays can be a bit stubborn
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

//---------------End of pld-lcd.h--------------------------------
