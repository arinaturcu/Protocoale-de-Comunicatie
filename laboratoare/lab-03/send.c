#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

char checksum(char msg[MSGSIZE]) {
	char result = msg[0] ^ msg[1];

	/* The last byte is reserved for checksum */
	for (int i = 2; i < MSGSIZE - 1; ++i) {
		result ^= msg[i];
	}

	return result;
}

void handle_response(int frame_number, int in_queue, char response[MSGSIZE], int *acks, int *nacks) {
	if (!strcmp(response, "NACK")) {
		printf("[SENDER] NACK for %d received. In queue: %d\n", frame_number, in_queue);
		(*nacks)++;
	} else if (!strcmp(response, "ACK")) {
		printf("[SENDER] ACK for %d received. In queue: %d\n", frame_number, in_queue);
		(*acks)++;
	} else {
		printf("WTF\n");
	}
}

int main(int argc, char *argv[])
{
	msg t;
	int i, res;
	int acks = 0, nacks = 0;
	
	printf("[SENDER] Starting.\n");	
	init(HOST, PORT);

	int window_size = (atoi(argv[1]) * 1000) / ((int) sizeof(t.payload) * 8);
	printf("[SENDER]: BDP = %d\n", window_size);
	int in_queue = 0;

	/* Send first window_size frames */
	for (i = 0; i < window_size; i++) {
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		t.len = MSGSIZE;
		t.payload[MSGSIZE - 1] = checksum(t.payload);
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}

		in_queue++;
		printf("[SENDER] Frame %d sent. In queue: %d\n", i, in_queue);
	}
	
	/* Send frames as the window is freed*/
	for (i = 0; i < COUNT - window_size; i++) {
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}

		in_queue--;
		handle_response(i, in_queue, t.payload, &acks, &nacks);

		/* cleanup msg */
		memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		t.len = MSGSIZE;
		t.payload[MSGSIZE - 1] = checksum(t.payload);
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}

		in_queue++;
		printf("[SENDER] Frame %d sent. In queue: %d\n", i + window_size, in_queue);
	}

	/* Receive ACK for remained frames */
	for (i = 0; i < window_size; ++i) {
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}

		in_queue--;
		handle_response(COUNT - (window_size - i), in_queue, t.payload, &acks, &nacks);
	}

	printf("[SENDER] Job done, all sent.\n");
	printf("ACKs: %d, NACKs: %d\n", acks, nacks);

	return 0;
}
