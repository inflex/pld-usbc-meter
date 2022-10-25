int i2c_write_register_byte( int busfd, uint8_t reg, uint8_t data );
int i2c_read_register(int busfd, uint8_t reg, unsigned char *buf, int bufsize);
int i2c_write_bus(int busfd, unsigned char *buf, int bufsize);
int i2c_read_bus(int busfd, unsigned char *buf, int bufsize);
int i2c_set_device(int busfd, int addr);
int i2c_close_bus( int busfd );
int i2c_open_bus( int bus ); 
int16_t i2c_twos2dec_16( unsigned char *raw );
int i2c_set_debug( int level );
int i2c_set_verbose( int level );