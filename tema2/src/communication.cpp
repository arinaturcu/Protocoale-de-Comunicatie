#include "communication.h"

static void reconnect(char* new_client_id,
					int newsockfd, sockaddr_in cli_addr,
					unordered_map<int, tcp_client> &active_clients,
					unordered_map<string, tcp_client> &all_clients,
					unordered_map<string, int> &active_ids_sockets,
					unordered_map<string, list<sub_message>> &store_and_forward) {
	tcp_client client = all_clients.at(new_client_id);

	active_clients.insert({newsockfd, client});
	active_ids_sockets.insert({client.id, newsockfd});

	all_clients.erase(new_client_id);
	all_clients.insert({new_client_id, client});

	cout << "New client " << new_client_id << " connected from " 
		 << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << ".\n"; 

	auto it = store_and_forward.find(client.id);
	if (it != store_and_forward.end()) {
		for (auto to_send = it->second.begin(); to_send != it->second.end(); to_send++) {
			send(client.socket, &(*to_send), sizeof(sub_message), 0);
		}

		store_and_forward.erase(it);
	}
}

static void add_new_client(int newsockfd, char new_client_id[],
						sockaddr_in cli_addr,
						unordered_map<int, tcp_client> &active_clients,
						unordered_map<string, tcp_client> &all_clients,
						unordered_map<string, int> &active_ids_sockets) {
	// adaug clientul in map-uri
	tcp_client client;
	client.socket = newsockfd;
	strcpy(client.id, new_client_id);

	all_clients.insert({new_client_id, client});
	active_clients.insert({newsockfd, client});
	active_ids_sockets.insert({new_client_id, newsockfd});

	cout << "New client " << new_client_id << " connected from " 
		 << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << ".\n"; 
}

void handle_connection_request(int sock_tcp, int &fd_max, fd_set &read_fds,
                               unordered_map<int, tcp_client> &active_clients,
							   unordered_map<string, tcp_client> &all_clients,
							   unordered_map<string, int> &active_ids_sockets,
							   unordered_map<string, list<sub_message>> &store_and_forward) {
	sockaddr_in cli_addr;
	char new_client_id[MAX_ID_LEN];

	int clilen = sizeof(cli_addr);
	memset(&cli_addr, 0, sizeof(sockaddr_in));
	int newsockfd = accept(sock_tcp, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
	DIE(newsockfd < 0, "accept");

	// primeste ID-ul clientului
	memset(new_client_id, 0, MAX_ID_LEN);
	int n = recv(newsockfd, new_client_id, MAX_ID_LEN, 0);
	DIE(n < 0, "recv");

	// verific daca clientul este deja conectat
	if (active_ids_sockets.find(new_client_id) != active_ids_sockets.end()) {
		cout << "Client " << new_client_id << " already connected.\n";
		shutdown(newsockfd, SHUT_RDWR);
		close(newsockfd);
		return;
	}

	// se adauga noul socket intors de accept() la multimea descriptorilor de citire
	FD_SET(newsockfd, &read_fds);
	if (newsockfd > fd_max) {
		fd_max = newsockfd;
	}

	// verific daca a mai fost conectat dar s-a deconectat intre timp
	if (all_clients.find(new_client_id) != all_clients.end()) { 
		reconnect(new_client_id, newsockfd, cli_addr, active_clients, all_clients, active_ids_sockets, store_and_forward);
		return;
	}
	
	// daca clientul este nou
	add_new_client(newsockfd, new_client_id, cli_addr, active_clients, all_clients, active_ids_sockets);
}

void handle_stdin_message(char buffer[], int sock_tcp) {
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN, stdin);

	if (strncmp(buffer, "exit", 4) == 0) {
		shutdown(sock_tcp, SHUT_RDWR);
		close(sock_tcp);
		exit(0);
	}
}

void handle_udp_message (char buffer[], int sock_udp, int i,
						struct sockaddr_in from_station,
						fd_set &read_fds,
						unordered_map<string, int> active_ids_sockets,
						unordered_map<string, list<subscription>> &topics_subs,
						unordered_map<string, list<sub_message>> &store_and_forward) {
	udp_message message;
	memset(&message, 0, sizeof(udp_message));

	memset(buffer, 0, BUFLEN);
	int szfrmst = sizeof(from_station);
	int n = recvfrom(sock_udp, buffer, UDP_LEN, 0, (struct sockaddr *)(&from_station), (socklen_t *)&szfrmst);
	DIE(n < 0, "recv");

	memcpy(&message.topic, buffer, TOPIC_LEN);
	memcpy(&message.data_type, buffer + TOPIC_LEN, 1);
	memcpy(&message.content, buffer + TOPIC_LEN + 1, CONTENT_LEN);

	auto it = topics_subs.find(message.topic);
	if (it == topics_subs.end()) {
		list<subscription> empty_list;
		topics_subs.insert({message.topic, empty_list});
		return;
	}

	for (auto s = it->second.begin(); s != it->second.end(); s++) {
		tcp_client client = s->client;

		// daca clientul e deconectat si are sf-ul 0
		if (active_ids_sockets.find(client.id) == active_ids_sockets.end() && s->sf == SF_OFF) {
			continue;
		}

		sub_message to_send;
		memset(&to_send, 0, sizeof(sub_message));
		memcpy(&to_send.from_station, &from_station, sizeof(sockaddr_in));
		memcpy(&to_send.message, &message, sizeof(udp_message));

		// daca clientul e conectat
		if (active_ids_sockets.find(client.id) != active_ids_sockets.end()) {
			send(client.socket, &to_send, sizeof(sub_message), 0);
			continue;
		}

		// daca clientul e deconectat si are sf-ul 1
		if (s->sf == SF_ON) {
			auto to_send_iter = store_and_forward.find(client.id);

			if (to_send_iter == store_and_forward.end()) {
				list<sub_message> to_send_list;
				to_send_list.insert(to_send_list.end(), to_send);
				store_and_forward.insert({client.id, to_send_list});
				continue;
			}

			to_send_iter->second.insert(to_send_iter->second.end(), to_send);
		}
	}
}

void handle_tcp_message(int i, fd_set &read_fds,
						unordered_map<int, tcp_client> &active_clients,
						unordered_map<string, tcp_client> &all_clients,
						unordered_map<string, int> &active_ids_sockets,
						unordered_map<string, list<subscription>> &topics_subs ) {
	client_request req;
	memset(&req, 0, sizeof(client_request));

	int n = recv(i, &req, sizeof(client_request), 0);
	DIE(n < 0, "recv");

	if (n == 0) {
		tcp_client client_to_delete;
		memset(&client_to_delete, 0, sizeof(tcp_client));
		client_to_delete = active_clients.at(i);

		// conexiunea s-a inchis
		cout << "Client " <<  client_to_delete.id << " disconnected.\n";
		shutdown(i, SHUT_RDWR);
		close(i);

		// se scoate din multimea de citire socketul inchis
		FD_CLR(i, &read_fds);
		active_ids_sockets.erase(active_clients.at(i).id);
		active_clients.erase(i);

		return;
	}

	auto it = topics_subs.find(req.topic);
	if (it == topics_subs.end()) {
		return;
	}

	char client_ID[MAX_ID_LEN];
	strcpy(client_ID, active_clients.at(i).id);

	if (req.req_type == SUBSCRIBE) {
		tcp_client client;

		// daca clientul e deja in lista, ii modific sf-ul daca este cazul
		for (auto s = it->second.begin(); s != it->second.end(); s++) {
			if (strcmp(s->client.id, client_ID) == 0) {
				s->sf = req.sf;
				return;
			}
		}

		// memcpy(&client, &(active_clients.at(i)), sizeof(client));
		it->second.insert(it->second.end(), {active_clients.at(i), req.sf});
		return;
	}

	if (req.req_type == UNSUBSCRIBE) {
		subscription sub_to_del;

		for (auto s = it->second.begin(); s != it->second.end(); s++) {
			if (strcmp(s->client.id, client_ID) == 0) {
				it->second.erase(s);
				break;
			}
		}
	}
}
