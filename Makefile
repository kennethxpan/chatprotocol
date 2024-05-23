CC = gcc
CFLAGS = -Wall -I.
LDFLAGS = -lngtcp2

all: server client

server: server.c util.c
	$(CC) $(CFLAGS) -o server server.c util.c $(LDFLAGS)

client: client.c util.c
	$(CC) $(CFLAGS) -o client client.c util.c $(LDFLAGS)

clean:
	rm -f server client
