CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS_SERVER =
LDFLAGS_CLIENT = -lncurses
LDFLAGS_TEST = -lncurses

# Targets
all: server client test

server: server.c header.h
	$(CC) $(CFLAGS) server.c -o server $(LDFLAGS_SERVER)

client: client.c ui.c header.h ui.h
	$(CC) $(CFLAGS) client.c ui.c -o client $(LDFLAGS_CLIENT)

test: test.c ui.c header.h ui.h
	$(CC) $(CFLAGS) test.c ui.c -o test $(LDFLAGS_TEST)

clean:
	rm -f server client test

.PHONY: all clean