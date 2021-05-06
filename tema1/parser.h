#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct route_table_entry {
	uint32_t prefix;
	uint32_t next_hop;
	uint32_t mask;
	int interface;
} __attribute__((packed));

int read_rtable(struct route_table_entry *rtable, FILE *rtable_file);