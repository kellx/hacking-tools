CC=gcc
CFLAGS=-Wall -I.
OBJS=rc4.o

all: $(OBJS) client.o server.o
	$(CC) -o client client.o $(OBJS) $(CFLAGS)
	$(CC) -o server server.o $(OBJS) $(CFLAGS)

clean:
	rm -f *.core core *.o client server
