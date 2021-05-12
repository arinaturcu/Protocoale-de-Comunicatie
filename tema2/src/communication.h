#pragma once

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

#define MAX_CLIENTS 10
#define MAX_ID_LEN 11
#define TOPIC_LEN 50
#define CONTENT_LEN 1500
#define UDP_LEN 1551
#define SUBSCRIBE '0'
#define UNSUBSCRIBE '1'
#define SF_OFF '0'
#define SF_ON '1'

using namespace std;

struct tcp_client {
  char id[MAX_ID_LEN];
  int socket;
};

struct udp_message {
  char topic[TOPIC_LEN + 1];
  char data_type;
  char content[CONTENT_LEN];
};

struct subscription {
  tcp_client client;
  char sf;
};

/**
 * req_type = 0 for subscribe
 * 			  1 for unsubscribe
 */
struct client_request {
  char topic[TOPIC_LEN + 1];
  char sf;
  char req_type;
};

struct sub_message {
  udp_message message;
  sockaddr_in from_station;
};

void handle_connection_request(int sock_tcp, int &fd_max, fd_set &read_fds,
                               unordered_map<int, tcp_client> &active_clients,
                               unordered_map<string, tcp_client> &all_clients,
                               unordered_map<string, int> &active_ids_sockets,
							   unordered_map<string, list<sub_message>> &store_and_forward);

void handle_stdin_message(char buffer[], int sock_tcp);

void handle_udp_message(char buffer[], int sock_udp, int i, struct sockaddr_in from_station,
						fd_set &read_fds, unordered_map<string, int> active_ids_sockets,
						unordered_map<string, list<subscription>> &topics_subs,
						unordered_map<string, list<sub_message>> &store_and_forward);

void handle_tcp_message(int i, fd_set &read_fds,
                        unordered_map<int, tcp_client> &active_clients,
                        unordered_map<string, tcp_client> &all_clients,
                        unordered_map<string, int> &active_ids_sockets,
                        unordered_map<string, list<subscription>> &topics_subs);
