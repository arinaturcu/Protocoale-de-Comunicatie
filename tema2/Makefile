CFLAGS = -Wall -g

all: server subscriber

server: server.cpp server_helper.cpp subscriber_helper.cpp
	g++ $(CFLAGS) server.cpp server_helper.cpp -o server

subscriber: subscriber.cpp subscriber_helper.cpp
	g++ $(CFLAGS) subscriber.cpp subscriber_helper.cpp -o subscriber

clean:
	rm -f server subscriber
