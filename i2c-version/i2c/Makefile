default: compass-switch

CC=gcc
LIBS=-lm
OBJS=keycode-decode.o i2c.o
CFLAGS=-Wall -O1

.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm *.o compass-switch

compass-switch: compass-switch.c $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) compass-switch.c -o compass-switch $(LIBS)

