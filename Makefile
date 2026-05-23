CC = gcc
CFLAGS = -Wall -Wextra -O2 -D_GNU_SOURCE
TARGETS = server client

all: $(TARGETS)

server: server.c
$(CC) $(CFLAGS) -o $@ $^

client: client.c
$(CC) $(CFLAGS) -o $@ $^

clean:
rm -f $(TARGETS)

run-server: server
@echo "Starting server..."
@./server
@echo "Server started as daemon"

run-client: client
./client 127.0.0.1 8080 1 mysecret123

install: $(TARGETS)
mkdir -p /usr/local/bin
cp $(TARGETS) /usr/local/bin/

.PHONY: all clean run-server run-client install
