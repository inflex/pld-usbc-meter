# 
# VERSION CHANGES
#

#BV=$(shell (git rev-list HEAD --count))
#BD=$(shell (date))
BV=1234
BD=today
SDLFLAGS=$(shell (sdl2-config --static-libs --cflags))
#CFLAGS=  -O0 -g -ggdb -DBUILD_VER="$(BV)" -DBUILD_DATE=\""$(BD)"\" -DFAKE_SERIAL=$(FAKE_SERIAL)
CFLAGS=  -O2 -DBUILD_VER="$(BV)" -DBUILD_DATE=\""$(BD)"\" -DFAKE_SERIAL=$(FAKE_SERIAL)
LIBS=-lSDL2_ttf
CC=gcc
GCC=g++

OBJ=pld-usbc-meter

OFILES=i2c.o

i2c.o: i2c.[ch]
	$(GCC) $(CFLAGS) -c i2c.c

default: $(OBJ)
	@echo
	@echo

pld-usbc-meter: pld-usbc-meter.cpp $(OFILES)
	@echo Build Release $(BV)
	@echo Build Date $(BD)
	${GCC} ${CFLAGS} $(COMPONENTS) $(OFILES) pld-usbc-meter.cpp $(SDLFLAGS) $(LIBS)  -o $(OBJ) 

strip:
	strip ${OBJ}

clean:
	rm -v *.o
	rm -v ${OBJ} 
