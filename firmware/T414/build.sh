#!/bin/sh

MCU=attiny414
NAME=main
UART=/dev/ttyUSB0


echo "Compiling..."
avr-gcc -Os -c -mmcu=${MCU} ${NAME}.c
if [ $? -ne 0 ]; then
	exit
	fi

echo "Linking..."
avr-gcc -mmcu=${MCU} -o ${NAME}.elf ${NAME}.o
if [ $? -ne 0 ]; then
	exit
	fi

# If you want to do some debugging you can uncomment
# these lines...
#avr-size ${NAME}.elf
#if [ $? -ne 0 ]; then
#	exit
#	fi
#avr-objdump -S ${NAME}.elf > ${NAME}.asm
#if [ $? -ne 0 ]; then
#	exit
#	fi
#cat ${NAME}.asm

echo "Converting to iHex..."
avr-objcopy -j .text -j .data -j .rodata -O ihex ${NAME}.elf ${NAME}.hex
if [ $? -ne 0 ]; then
	exit
	fi

echo "Writing using pyupdi..."
pymcuprog --verify -d ${MCU} -t uart -u ${UART} write -f ${NAME}.hex --erase
if [ $? -ne 0 ]; then
	exit
	fi


#echo "Exiting out of programming mode..."
#pymcuprog -d ${MCU} -t uart -u ${UART} reset
#if [ $? -ne 0 ]; then
#	exit
#	fi


echo "Completed"


