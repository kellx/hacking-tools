CC=gcc
CFLAGS=-Wall -fomit-frame-pointer -I. -O2 
OBJS=banscan.o list.o init.o synsweep.o enumeration.o readline.o
LIBS=-lpcap

banscan: $(OBJS)
	$(CC) $(CFLAGS) -o banscan $(OBJS) $(LIBS)

clean:
	rm -f *.o *.core core banscan
