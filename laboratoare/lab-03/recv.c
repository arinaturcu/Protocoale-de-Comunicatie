#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

char checksum(char msg[MSGSIZE]) {
	char result = msg[0] ^ msg[1];

	/* The last byte is reserved for checksum */
	for (int i = 2; i < MSGSIZE - 1; ++i) {
		result ^= msg[i];
	}

	return result;
}

int main(void)
{
	msg r;
	int i, res;
	
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	
	for (i = 0; i < COUNT; i++) {
		/* wait for message */
		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}

		printf("[RECEIVER] Message %d received.\n", i);


		if (r.payload[MSGSIZE - 1] == checksum(r.payload)) 
		{
			/* send dummy ACK */
			sprintf(r.payload, "ACK");
			res = send_message(&r);
			if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
			printf("[RECEIVER] Sent ACK for %d.\n", i);
		} 
		else 
		{
			sprintf(r.payload, "NACK");
			res = send_message(&r);
			if (res < 0) {
				perror("[RECEIVER] Send NACK error. Exiting.\n");
				return -1;
			}
			printf("[RECEIVER] Sent NACK for %d.\n", i);
		}
	}

	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}
