CC=g++

client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) -g -std=c++11 -I/home/linuxbrew/.linuxbrew/Cellar/nlohmann-json/3.9.1_1/include -o client client.cpp requests.cpp helpers.cpp buffer.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
