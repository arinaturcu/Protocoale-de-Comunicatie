#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char **argv)
{
	msg r;
	int size;
	char filename[32];
	int files_number;
	init(HOST, PORT);

	// Recieve the number of files
	if (recv_message(&r) < 0) {
		perror("Receive message");
		return -1;
	}

	printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);
	files_number = r.len;
	
	for (int i = 0; i < files_number; ++i) {
		// Recieve file name
		if (recv_message(&r) < 0) {
			perror("Receive message");
			return -1;
		}

		printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);
		strcpy(filename, r.payload);

		// Send ACK
		sprintf(r.payload, "%s", "ACK");
		r.len = strlen(r.payload) + 1;
		send_message(&r);
		printf("[recv] ACK sent\n");

		// Recieve file size
		if (recv_message(&r) < 0) {
			perror("Receive message");
			return -1;
		}

		printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);

		size = r.len;

		// Send ACK
		sprintf(r.payload, "%s", "ACK");
		r.len = strlen(r.payload) + 1;
		send_message(&r);
		printf("[recv] ACK sent\n");

		// Create file
		char copy_filename[38];
		strcpy(copy_filename, "copy_");
		strcat(copy_filename, filename);

		int copy_file = open(copy_filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
		if (copy_file < 0) {
			perror("[recv]");
			return -1;
		}

		// Receive content
		int count;
		if (size % MAX_LEN == 0) {
			count = size / MAX_LEN;
		} else {
			count = size / MAX_LEN + 1; 
		}

		while (count > 0) {
			if (recv_message(&r) < 0) {
				perror("Receive message");
				return -1;
			}

			printf("[recv] Got msg with payload length: <%d>, sending ACK...\n", r.len);
			int copied = write(copy_file, r.payload, r.len);
			if (copied < 0) {
				perror("[recv]");
				return -1;
			}

			// Send ACK
			sprintf(r.payload, "%s", "ACK");
			r.len = strlen(r.payload) + 1;
			send_message(&r);
			printf("[recv] ACK sent\n");
			
			count--;
		}

		close(copy_file);
	}

	return 0;
}
