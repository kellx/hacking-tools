CC=gcc
CFLAGS=-Wall -g -I.
OBJS=proxy.o main.o

all: $(OBJS)	
	$(CC) $(CFLAGS)   -o proxy $(OBJS)

clean:
	rm -f *.core $(OBJS) proxy
