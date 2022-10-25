#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/limits.h>
//#include <linux/uinput.h>
//#include <dirent.h>

#include "i2c.h"

#define I2C_DBG if (debug)
static int debug = 0;
static int verbose = 0;

int i2c_set_debug( int level ) {
	debug = level;
	return debug;
}

int i2c_set_verbose( int level ) {
	verbose = level;
	return verbose;
}

int16_t i2c_twos2dec_16( unsigned char *raw ) {

	/*
	 * Convert 2's complement big-endian 16-bit integer
	 * in to a 16-bit int (intel)
	 *
	 * We have to first see if the high bit is set to 
	 * indicate a negative number, if it is, we need
	 * to invert the bits in both bytes, assemble in to
	 * a little-endian integer, and *then* add 1 to the
	 * result and cast back in to a 16-bit integer.
	 *
	 * For non-negative numbers, we can just assemble 
	 * the two bytes in to  little-endian form and 
	 * cast to 16-bit int.
	 *
	 */

	uint32_t tmp = 0;
	int16_t result = 0;

	if ((raw[0] & 0x80) == 0x80) {
		// high bit is set, negative number
		result = -(((raw[0]<<8|raw[1]) ^0xffff) +1); 
		/*
		 * Below is the original code, kept in here so others
		 * can know what's going on with the 'compact' version
		 * above
		 *
		raw[0] = ~(raw[0]);
		raw[1] = ~(raw[1]);
		tmp = (raw[0] << 8) | raw[1];
		result = (int16_t)(tmp +1);
		result = -result;

		 *
		 */

	} else {
		// no high bit set, just assemble normally
		result = (int16_t)((raw[0] << 8) | raw[1]);
	}

	return result;
}

int i2c_open_bus( int bus ) {
	int file;
	char filename[PATH_MAX];
	snprintf(filename, sizeof(filename), "/dev/i2c-%d", bus);
	if ((file = open(filename, O_RDWR)) < 0) {
		I2C_DBG fprintf(stderr,"Failed to open the i2c bus (%s)\n", strerror(errno));
	}

	return file;
}

int i2c_close_bus( int busfd ) {
	return close(busfd);
}


int i2c_set_device(int busfd, int addr) {
	int ret = ioctl(busfd, I2C_SLAVE, addr);
	if (ret < 0) {
		I2C_DBG fprintf(stderr,"Failed to acquire bus access and/or talk to slave. (%s)\n", strerror(errno));
	}

	return ret;
}

int i2c_read_bus(int busfd, unsigned char *buf, int bufsize) {
	return read(busfd, buf, bufsize);
}

int i2c_write_bus(int busfd, unsigned char *buf, int bufsize) {
	return write(busfd, buf, bufsize);
}

int i2c_read_register(int busfd, uint8_t reg, unsigned char *buf, int bufsize) {
	int ret;

	ret = i2c_write_bus(busfd, &reg, 1);
	if (ret < 0) {
		I2C_DBG fprintf(stderr,"Failed to write register address %02x (%s)\n", reg, strerror(errno));
		return ret;
	}

	return i2c_read_bus(busfd, buf, bufsize);
}


int i2c_write_register_byte( int busfd, uint8_t reg, uint8_t data ) {
	/*
	 * Write a byte value to a register with an 8-bit address
	 *
	 */
	unsigned char buf[2];
	ssize_t ws = 0;

	buf[0] = reg;
	buf[1] = data;
	ws = i2c_write_bus(busfd, buf, 2);
	if (ws != 2) {
		I2C_DBG fprintf(stderr,"Couldn't write register address [ %x ] (%s)\n", reg, strerror(errno)); 
	}

	return ws;
}


