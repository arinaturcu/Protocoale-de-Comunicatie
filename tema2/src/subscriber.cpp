#include "server_helper.h"

#define TYPE_MAX_LEN 11

void usage(char *file)
{
	fprintf(stderr, "Usage: %s id_client server_address server_port\n", file);
	exit(0);
}

void subscribe(int sockfd, char *topic, char sf) {
	client_request s;
	memset(&s, 0, sizeof(client_request));
	s.req_type = SUBSCRIBE;
	s.sf = sf;
	memset(&s.topic, 0, TOPIC_LEN);
	memcpy(&s.topic, topic, strlen(topic));

	int n = send(sockfd, &s, sizeof(client_request), 0);
	DIE(n < 0, "send");

	printf("Subscribed to topic.\n");
}

void unsubscribe(int sockfd, char *topic) {
	client_request s;
	memset(&s, 0, sizeof(client_request));
	s.req_type = UNSUBSCRIBE;
	s.sf = 0;
	memset(&s.topic, 0, TOPIC_LEN);
	memcpy(&s.topic, topic, strlen(topic));

	int n = send(sockfd, &s, sizeof(client_request), 0);
	DIE(n < 0, "send");

	printf("Unsubscribed from topic.\n");
}

void print_output(sub_message response) {
	// ip si port
	cout << inet_ntoa(response.from_station.sin_addr) << ":" << (int) ntohs(response.from_station.sin_port) << " - ";
	
	// topic
	cout << response.message.topic << " - ";

	// data_type
	char buffer[CONTENT_LEN + 1];
	memset(buffer, 0, CONTENT_LEN + 1);
	float res;
	short tmp;

	switch (response.message.data_type) {
	case 0:
		uint32_t int_x;
		memcpy(&int_x, response.message.content + 1, sizeof(int_x));
		int_x = ntohl(int_x);
		
		if (response.message.content[0] == 1) {
			cout << "INT - -" << int_x << endl;
			break;
		}
		
		cout << "INT - " << int_x << endl;
		break;
	
	case 1:
		uint16_t short_x;
		memcpy(&short_x, response.message.content, sizeof(uint16_t));
		short_x = ntohs(short_x);

		cout << "SHORT_REAL - " << fixed << setprecision(2) << (float) short_x / 100 << endl;
		break;

	case 2:
		uint32_t float_x;
		uint8_t power_of_ten;

		memcpy(&float_x, response.message.content + 1, sizeof(float_x));
		memcpy(&power_of_ten, response.message.content + 1 + sizeof(float_x), sizeof(power_of_ten));
		
		float_x = ntohl(float_x);

		res = float_x;
		tmp = power_of_ten;
		while(tmp > 0) {
			res = res / 10;
			tmp--;
		}

		if (response.message.content[0] == 1) {
			cout << "FLOAT - -" << fixed << setprecision(power_of_ten) << res << endl;
			break;
		}

		cout << "FLOAT - " << fixed << setprecision(power_of_ten) << res << endl;
		break;
	
	case 3:
		memcpy(buffer, response.message.content, CONTENT_LEN);
		cout  << "STRING - " << buffer << endl;
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");
	int on = 1;
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

    // adauga mesaj pt client ID
	memcpy(buffer, argv[1], strlen(argv[1]) + 1);
	n = send(sockfd, buffer, strlen(buffer), 0);
	DIE(n < 0, "send");

	// se adauga file descriptor-ul 0 (STDIN)
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);

	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			if (strncmp(buffer, "exit", 4) == 0) {
                shutdown(sockfd, SHUT_RDWR);
                close(sockfd);
				break;
			}

			char *command;
			command = strtok(buffer, " ");

			if (strncmp(command, "subscribe", 10) == 0) {
				char *topic = strtok(NULL, " ");
				char *sf = strtok(NULL, " \n"); 
				subscribe(sockfd, topic, sf[0]);
				continue;
			}

			if (strncmp(command, "unsubscribe", 12) == 0) {
				char *topic = strtok(NULL, " \n");
				unsubscribe(sockfd, topic);
				continue;
			}
		} 

		if (FD_ISSET(sockfd, &tmp_fds)) {
			sub_message response;
			memset(&response, 0, sizeof(sub_message));

			int bytes_recv = recv(sockfd, &response, sizeof(sub_message), 0);
			DIE(bytes_recv < 0, "receive");

            if (bytes_recv == 0) {
				shutdown(sockfd, SHUT_RDWR);
                close(sockfd);
                break;
            }

			print_output(response);
		}
	}

	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);

	return 0;
}