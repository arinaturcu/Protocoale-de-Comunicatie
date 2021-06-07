#include <bits/stdc++.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "helpers.h"

using namespace std;

void handle_stdin_message(int sockfd) {
    string buffer;
	getline(cin, buffer);

    if (buffer == "exit") {
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(0);
    }

	if (strncmp(buffer.c_str(), "MSG", 3) == 0) {
		message to_send;
		memset(&to_send, 0, sizeof(message));

		char *nr = (char *)calloc(MAX_LEN, sizeof(char));
		nr = strtok((char *)buffer.c_str() + 4, " ");
		
		to_send.nr_dest = atoi(nr);
		strcpy(to_send.msg, buffer.c_str() + 6);
		
		int n = send(sockfd, &to_send, sizeof(message), 0);
		DIE(n < 0, "send");
		return;
	}

	if (strncmp(buffer.c_str(), "LIST", 4) == 0) {
		message to_send;
		memset(&to_send, 0, sizeof(message));

		to_send.nr_dest = -1;
		strcpy(to_send.msg, "LIST");

		int n = send(sockfd, &to_send, sizeof(message), 0);
		DIE(n < 0, "send");
		return;
	}
}

void handle_server_message(int sockfd, fd_set &read_fds) {
    char buffer[BUFLEN];
    memset(buffer, 0, BUFLEN);

    int bytes_recv = recv(sockfd, buffer, BUFLEN, 0);
    DIE(bytes_recv < 0, "receive");

    if (bytes_recv == 0) {
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
		FD_CLR(sockfd, &read_fds);
		exit(0);
        return;
    }

    cout << buffer << endl;
}

void usage(char *file)
{
	fprintf(stderr, "Usage: %s id_client server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	char buffer[BUFLEN];
	
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");
	int on = 1;
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

    // adauga mesaj pt client ID
	memcpy(buffer, argv[1], strlen(argv[1]) + 1);
	n = send(sockfd, buffer, strlen(buffer), 0);
	DIE(n < 0, "send");

	// se adauga file descriptor-ul 0 (STDIN)
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);

	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
			handle_stdin_message(sockfd);
			continue;
		} 

		if (FD_ISSET(sockfd, &tmp_fds)) {
			handle_server_message(sockfd, read_fds);
		}
	}

	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);

	return 0;
}