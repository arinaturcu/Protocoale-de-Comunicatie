CFLAGS = -Wall -g

all: server client

server: server.cpp
	g++ $(CFLAGS) server.cpp -o server

client: client.cpp
	g++ $(CFLAGS) client.cpp -o client

clean:
	rm -f server client
