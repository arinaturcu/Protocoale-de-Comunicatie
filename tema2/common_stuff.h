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

struct client_request {
	char topic[TOPIC_LEN + 1];
	char sf;
	char req_type;
};

struct sub_message {
    udp_message message;
    sockaddr_in from_station;
};
