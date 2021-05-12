#pragma once
#include "common_stuff.h"
#include "subscriber_helper.h"

#define TYPE_MAX_LEN 11

void subscribe(int sockfd, char *topic, char sf);

void unsubscribe(int sockfd, char *topic);

void print_output(sub_message response);

void handle_stdin_message(int sockfd);

void handle_server_message(int sockfd);
