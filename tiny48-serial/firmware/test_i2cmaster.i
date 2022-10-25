# 1 "test_i2cmaster.c"
# 1 "/home/pld/development/pld/pld-usbc-meter/serial-version/firmware//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "test_i2cmaster.c"
# 14 "test_i2cmaster.c"
# 1 "/usr/lib/avr/include/avr/io.h" 1 3
# 99 "/usr/lib/avr/include/avr/io.h" 3
# 1 "/usr/lib/avr/include/avr/sfr_defs.h" 1 3
# 126 "/usr/lib/avr/include/avr/sfr_defs.h" 3
# 1 "/usr/lib/avr/include/inttypes.h" 1 3
# 37 "/usr/lib/avr/include/inttypes.h" 3
# 1 "/usr/lib/gcc/avr/5.4.0/include/stdint.h" 1 3 4
# 9 "/usr/lib/gcc/avr/5.4.0/include/stdint.h" 3 4
# 1 "/usr/lib/avr/include/stdint.h" 1 3 4
# 125 "/usr/lib/avr/include/stdint.h" 3 4

# 125 "/usr/lib/avr/include/stdint.h" 3 4
typedef signed int int8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef signed int int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef signed int int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));

typedef signed int int64_t __attribute__((__mode__(__DI__)));
typedef unsigned int uint64_t __attribute__((__mode__(__DI__)));
# 146 "/usr/lib/avr/include/stdint.h" 3 4
typedef int16_t intptr_t;




typedef uint16_t uintptr_t;
# 163 "/usr/lib/avr/include/stdint.h" 3 4
typedef int8_t int_least8_t;




typedef uint8_t uint_least8_t;




typedef int16_t int_least16_t;




typedef uint16_t uint_least16_t;




typedef int32_t int_least32_t;




typedef uint32_t uint_least32_t;







typedef int64_t int_least64_t;






typedef uint64_t uint_least64_t;
# 217 "/usr/lib/avr/include/stdint.h" 3 4
typedef int8_t int_fast8_t;




typedef uint8_t uint_fast8_t;




typedef int16_t int_fast16_t;




typedef uint16_t uint_fast16_t;




typedef int32_t int_fast32_t;




typedef uint32_t uint_fast32_t;







typedef int64_t int_fast64_t;






typedef uint64_t uint_fast64_t;
# 277 "/usr/lib/avr/include/stdint.h" 3 4
typedef int64_t intmax_t;




typedef uint64_t uintmax_t;
# 10 "/usr/lib/gcc/avr/5.4.0/include/stdint.h" 2 3 4
# 38 "/usr/lib/avr/include/inttypes.h" 2 3
# 77 "/usr/lib/avr/include/inttypes.h" 3
typedef int32_t int_farptr_t;



typedef uint32_t uint_farptr_t;
# 127 "/usr/lib/avr/include/avr/sfr_defs.h" 2 3
# 100 "/usr/lib/avr/include/avr/io.h" 2 3
# 426 "/usr/lib/avr/include/avr/io.h" 3
# 1 "/usr/lib/avr/include/avr/iotn45.h" 1 3
# 427 "/usr/lib/avr/include/avr/io.h" 2 3
# 627 "/usr/lib/avr/include/avr/io.h" 3
# 1 "/usr/lib/avr/include/avr/portpins.h" 1 3
# 628 "/usr/lib/avr/include/avr/io.h" 2 3

# 1 "/usr/lib/avr/include/avr/common.h" 1 3
# 630 "/usr/lib/avr/include/avr/io.h" 2 3

# 1 "/usr/lib/avr/include/avr/version.h" 1 3
# 632 "/usr/lib/avr/include/avr/io.h" 2 3






# 1 "/usr/lib/avr/include/avr/fuse.h" 1 3
# 239 "/usr/lib/avr/include/avr/fuse.h" 3
typedef struct
{
    unsigned char low;
    unsigned char high;
    unsigned char extended;
} __fuse_t;
# 639 "/usr/lib/avr/include/avr/io.h" 2 3


# 1 "/usr/lib/avr/include/avr/lock.h" 1 3
# 642 "/usr/lib/avr/include/avr/io.h" 2 3
# 15 "test_i2cmaster.c" 2
# 1 "i2cmaster.h" 1
# 102 "i2cmaster.h"

# 102 "i2cmaster.h"
extern void i2c_init(void);






extern void i2c_stop(void);
# 119 "i2cmaster.h"
extern unsigned char i2c_start(unsigned char addr);
# 129 "i2cmaster.h"
extern unsigned char i2c_rep_start(unsigned char addr);
# 139 "i2cmaster.h"
extern void i2c_start_wait(unsigned char addr);
# 148 "i2cmaster.h"
extern unsigned char i2c_write(unsigned char data);






extern unsigned char i2c_readAck(void);





extern unsigned char i2c_readNak(void);
# 172 "i2cmaster.h"
extern unsigned char i2c_read(unsigned char ack);
# 16 "test_i2cmaster.c" 2





int main(void)
{
    unsigned char ret;


    
# 26 "test_i2cmaster.c" 3
   (*(volatile uint8_t *)((0x17) + 0x20)) 
# 26 "test_i2cmaster.c"
         = 0xff;
    
# 27 "test_i2cmaster.c" 3
   (*(volatile uint8_t *)((0x18) + 0x20)) 
# 27 "test_i2cmaster.c"
         = 0xff;

    i2c_init();


    ret = i2c_start(0xA2 +0);
    if ( ret ) {

        i2c_stop();
        
# 36 "test_i2cmaster.c" 3
       (*(volatile uint8_t *)((0x18) + 0x20))
# 36 "test_i2cmaster.c"
            =0x00;
    }else {

        i2c_write(0x05);
        i2c_write(0x75);
        i2c_stop();



        i2c_start_wait(0xA2 +0);
        i2c_write(0x05);
        i2c_rep_start(0xA2 +1);
        ret = i2c_readNak();
        i2c_stop();

        
# 51 "test_i2cmaster.c" 3
       (*(volatile uint8_t *)((0x18) + 0x20)) 
# 51 "test_i2cmaster.c"
             = ~ret;



        i2c_start_wait(0xA2 +0);
        i2c_write(0x00);
        i2c_write(0x70);
        i2c_write(0x71);
        i2c_write(0x72);
        i2c_write(0x74);
        i2c_stop();



        i2c_start_wait(0xA2 +0);
        i2c_write(0x00);
        i2c_rep_start(0xA2 +1);
        ret = i2c_readAck();
        ret = i2c_readAck();
        ret = i2c_readAck();
        ret = i2c_readNak();
        i2c_stop();

        
# 74 "test_i2cmaster.c" 3
       (*(volatile uint8_t *)((0x18) + 0x20)) 
# 74 "test_i2cmaster.c"
             = ~ret;
    }

    for(;;);
}
