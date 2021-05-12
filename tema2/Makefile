CFLAGS = -Wall -g

# Portul pe care asculta serverul
PORT = 

# Adresa IP a serverului
IP_SERVER = 

all: server subscriber

# Compileaza server.c
server: ./src/server.cpp ./src/communication.cpp
	g++ -g ./src/server.cpp ./src/communication.cpp -o server

# Compileaza subscriber.c
subscriber: ./src/subscriber.cpp
	g++ -g ./src/subscriber.cpp -o subscriber

.PHONY: clean run_server run_client

# Ruleaza serverul
run_server:
	./src/server ${PORT} ${FILE_SERVER}

# Ruleaza clientul 	
run_subscriber:
	./src/client ${IP_SERVER} ${PORT} ${FILE_CLIENT}

clean:
	rm -f server subscriber
	# sudo fuser -k 12345/udp