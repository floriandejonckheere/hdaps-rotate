# Makefile for hdaps-rotate

CFLAGS		= -W -Wall
LIBRARIES	= 

all: hdaps-rotate

hdaps-rotate: hdaps-rotate.c
	$(CC) $(CFLAGS) $(LIBRARIES) -o hdaps-rotate hdaps-rotate.c

clean:
	rm -f hdaps-rotate *.o
