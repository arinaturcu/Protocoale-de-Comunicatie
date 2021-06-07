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

struct tcp_client {
	int number;
	char nickname[MAX_LEN];
};

void handle_stdin_message(int sock_tcp, int fd_max, fd_set read_fds) {
	string buffer;
	getline(cin, buffer);

	if (buffer == "exit") {
		for (int i = 1; i <= fd_max; ++i) {
			if (FD_ISSET(i, &read_fds)) {
				shutdown(i, SHUT_RDWR);
				close(i);
			}
		}
		
		shutdown(sock_tcp, SHUT_RDWR);
		close(sock_tcp);
		exit(0);
	}
}

static void add_new_client(int newsockfd, char new_client_nickname[],
						sockaddr_in cli_addr,
						unordered_map<string, tcp_client> &active_clients,
						unordered_map<string, tcp_client> &all_clients,
						unordered_map<int, int> &active_nrs_sockets,
						unordered_map<int, tcp_client> &active_socket_client,
						int &curr_number) {
	// adaug clientul in map-uri
	tcp_client client;
	client.number = curr_number++;
	strcpy(client.nickname, new_client_nickname);

	active_clients.insert({client.nickname, client});
	all_clients.insert({client.nickname, client});
	active_nrs_sockets.insert({client.number, newsockfd});
	active_socket_client.insert({newsockfd, client});

	cout << "S-a conectat un nou client, pe socket-ul " << newsockfd << ".\n";
}

void handle_connection_request(int sock_tcp, int &fd_max, fd_set &read_fds,
                               unordered_map<string, tcp_client> &active_clients,
							   unordered_map<string, tcp_client> &all_clients,
							   unordered_map<int, int> &active_nrs_sockets,
							   unordered_map<int, tcp_client> &active_socket_client,
							   int &curr_id) {
	int clilen = sizeof(sockaddr_in);
	sockaddr_in cli_addr;
	memset(&cli_addr, 0, sizeof(sockaddr_in));

	int newsockfd = accept(sock_tcp, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
	DIE(newsockfd < 0, "accept");

	// primeste nickname-ul clientului
	char new_client_nickname[MAX_LEN];
	memset(new_client_nickname, 0, MAX_LEN);

	int n = recv(newsockfd, new_client_nickname, MAX_LEN, 0);
	DIE(n < 0, "recv");

	// verific daca clientul este deja conectat
	if (active_clients.find(new_client_nickname) != active_clients.end()) {
		cout << "Client " << new_client_nickname << " aeste deja conectat.\n";
		shutdown(newsockfd, SHUT_RDWR);
		close(newsockfd);
		return;
	}

	if (all_clients.find(new_client_nickname) != all_clients.end()) {
		cout << "Client " << new_client_nickname << " s-a reconectat.\n";
		tcp_client client = all_clients.at(new_client_nickname);

		active_clients.insert({client.nickname, client});
		active_nrs_sockets.insert({client.number, newsockfd});
		active_socket_client.insert({newsockfd, client});
		return;
	}

	// se adauga noul socket intors de accept() la multimea descriptorilor de citire
	FD_SET(newsockfd, &read_fds);
	if (newsockfd > fd_max) {
		fd_max = newsockfd;
	}
	
	// daca clientul este nou
	add_new_client(newsockfd, new_client_nickname, cli_addr, active_clients, 
				   all_clients, active_nrs_sockets, active_socket_client, curr_id);
}

void handle_tcp_message(int i, fd_set &read_fds,
						unordered_map<string, tcp_client> &active_clients,
						unordered_map<string, tcp_client> &all_clients,
						unordered_map<int, int> &active_nrs_sockets,
						unordered_map<int, tcp_client> &active_socket_client) {
	message received;
	memset(&received, 0, sizeof(message));

	int n = recv(i, &received, BUFLEN, 0);
	DIE(n < 0, "[sender]: recv");

	// clientul se deconecteaza
	if (n == 0) {
		tcp_client client_to_delete;
		// memset(&client_to_delete, 0, sizeof(tcp_client));
		client_to_delete = active_socket_client.at(i);

		// conexiunea s-a inchis
		cout << "Client " <<  client_to_delete.nickname << " disconnected.\n";
		shutdown(i, SHUT_RDWR);
		close(i);

		// se scoate din multimea de citire socketul inchis
		FD_CLR(i, &read_fds);

		// se scoate din map-urile de clienti activi
		active_nrs_sockets.erase(client_to_delete.number);
		active_clients.erase(client_to_delete.nickname);
		active_socket_client.erase(i);

		return;
	}

	if (received.nr_dest == -1) {
		// LIST
		char buffer[BUFLEN];
		memset(buffer, 0, BUFLEN);

		for (auto entry : active_clients) {
			strcat(buffer, to_string(entry.second.number).c_str());
			strcat(buffer, " - ");
			strcat(buffer, entry.first.c_str());
			strcat(buffer, "\n");
		}

		int sent = send(i, buffer, BUFLEN, 0);
		DIE(sent < 0, "[server]: send");

	} else {
		if (active_nrs_sockets.find(received.nr_dest) == active_nrs_sockets.end()) {
			cout << "Clientul nu numarul " << received.nr_dest << " nu este activ!";
			return; 
		}

		int send_to_socket = active_nrs_sockets.at(received.nr_dest);
		int sent = send(send_to_socket, received.msg, MAX_LEN, 0);
		DIE(sent < 0, "[server]: send");
	}
}

void usage(char *file) {
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		usage(argv[0]);
	}

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	struct sockaddr_in from_station;

	fd_set read_fds;
	FD_ZERO(&read_fds);

	// Deschidere socketi
	int sock_tcp = socket(PF_INET, SOCK_STREAM, 0); // TCP
	DIE(sock_tcp == -1, "Open TCP socket");

	// setare struct sockaddr_in pentru a asculta pe portul respectiv
	from_station.sin_family = AF_INET;
	from_station.sin_port = htons(atoi(argv[1]));
	from_station.sin_addr.s_addr = INADDR_ANY;

	// bind
	int rs_tcp = bind(sock_tcp, (struct sockaddr *)(&from_station), sizeof(from_station));
	DIE(rs_tcp < 0, "Open bind");

	// listen
	int ret_tcp = listen(sock_tcp, 5);
	DIE(ret_tcp < 0, "listen");

	// key: nickname, value: client
	unordered_map<string, tcp_client> active_clients;
	// key: socket, value: client
	unordered_map<int, tcp_client> active_socket_client;
	// key: client number, socket conexiune
	unordered_map<int, int> active_nrs_sockets;
	// key: nickname, client
	unordered_map<string, tcp_client> all_clients;

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sock_tcp, &read_fds);

	int fd_max = sock_tcp;
	int curr_id = 1;

	while (1) {
		fd_set tmp_fds = read_fds;
		int ret = select(fd_max + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (int i = 0; i <= fd_max; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == 0) {
					handle_stdin_message(sock_tcp, fd_max, read_fds);
					continue;
				}

				if (i == sock_tcp) {					
					handle_connection_request(sock_tcp, fd_max, read_fds,
						active_clients, all_clients, active_nrs_sockets, active_socket_client, curr_id);
					continue;
				}
				
				handle_tcp_message(i, read_fds, active_clients, all_clients, 
					active_nrs_sockets, active_socket_client);
			}
		}
	}

	// inchidere socketi
	for (int i = 1; i <= fd_max; ++i) {
		if (FD_ISSET(i, &read_fds)) {
			shutdown(i, SHUT_RDWR);
			close(i);
		}
	}

	shutdown(sock_tcp, SHUT_RDWR);
	close(sock_tcp);

	return 0;
}
