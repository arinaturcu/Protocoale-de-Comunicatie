#pragma once
#include "common_stuff.h"

void handle_connection_request(
    int sock_tcp, int &fd_max, fd_set &read_fds,
    unordered_map<int, tcp_client> &active_clients,
    unordered_map<string, tcp_client> &all_clients,
    unordered_map<string, int> &active_ids_sockets,
    unordered_map<string, list<sub_message>> &store_and_forward);

void handle_stdin_message(char buffer[], int sock_tcp, int fd_max, fd_set read_fds);

void handle_udp_message(
    char buffer[], int sock_udp, int i, struct sockaddr_in from_station,
    fd_set &read_fds, unordered_map<string, int> active_ids_sockets,
    unordered_map<string, list<subscription>> &topics_subs,
    unordered_map<string, list<sub_message>> &store_and_forward);

void handle_tcp_message(int i, fd_set &read_fds,
                        unordered_map<int, tcp_client> &active_clients,
                        unordered_map<string, tcp_client> &all_clients,
                        unordered_map<string, int> &active_ids_sockets,
                        unordered_map<string, list<subscription>> &topics_subs);
