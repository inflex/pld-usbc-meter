CC=avr-gcc 
UC=attiny45
PROGRAMER=usbtiny
CFLAGS=-Wall -Os  -mmcu=$(UC)

OBJ2HEX=avr-objcopy 
TARGET=pld-usbc-meter-serial-firmware
OBJECTS=i2cmaster.o

program : $(TARGET).hex
#avrdude -F  -C /etc/avrdude.conf -e -v -p $(UC) -c usbtiny -U flash:w:$(TARGET).hex  

default: lom

%.obj : %.o
	$(CC) $(CFLAGS) $< -o $@

%.hex : %.obj
	$(OBJ2HEX) -R .eeprom -O ihex $< $@

clean :
	rm -f *.hex *.o *.obj

pld-usbc-meter-serial-firmware: pld-usbc-meter-serial-firmware.c $(OBJECTS)
	$(CC) $(CFLAGS) pld-usbc-meter-serial-firmware.c $(OBJECTS) -o pld-usbc-meter-serial-firmware.obj


burn: pld-usbc-meter-serial-firmware.hex
	avrdude  -e -E noreset -v -V -p $(UC) -B 3 -c $(PROGRAMER) -U flash:w:$(TARGET).hex  
	
