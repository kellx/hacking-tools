CC=gcc
CFLAGS=-Wall -fomit-frame-pointer -O2 -I.
OBJS=swipe.o remove.o wipe.o

swipe: $(OBJS)
	$(CC) $(CFLAGS)   -o swipe $(OBJS)
	strip swipe
clean:
	rm -f *.core *.o core swipe
