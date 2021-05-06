#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

void handle_response(msg t) {
	if (recv_message(&t) < 0) {
		perror("Recieve error");
		exit(-1);
	} else {
		printf("[send] Got reply with payload: %s\n", t.payload);
	}
}

int main(int argc, char **argv)
{
	init(HOST, PORT);
	msg t;

	t.len = argc - 1;
	strcpy(t.payload, "number of files");
	send_message(&t);
	printf("[send] Message sent.\n");

	for (int i = 1; i < argc; ++i) {
		int source = open(argv[i], O_RDONLY);
		if (source < 0) {
			perror("[SEND]");
		}

		// Send file name
		strcpy(t.payload, argv[i]);
		t.len = strlen(t.payload) + 1;

		send_message(&t);
		printf("[send] Message sent.\n");

		handle_response(t);

		// Send file size
		int size = lseek(source, 0, SEEK_END);
		t.len = size;
		memcpy(t.payload, "file size", 10);
		lseek(source, 0, SEEK_SET);

		send_message(&t);
		printf("[send] Message sent.\n");

		handle_response(t);

		int count;

		while ((count = read(source, t.payload, MAX_LEN))) {
			t.len = count;
			send_message(&t);
			printf("[send] Message sent.\n");

			handle_response(t);
		}
	}

	return 0;
}
