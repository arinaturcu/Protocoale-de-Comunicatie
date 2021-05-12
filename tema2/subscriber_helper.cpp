#include "common_stuff.h"
#include "subscriber_helper.h"

void subscribe(int sockfd, char *topic, char sf) {
	client_request s;
	memset(&s, 0, sizeof(client_request));
	memcpy(&s.topic, topic, strlen(topic));
	s.req_type = SUBSCRIBE;
	s.sf = sf;

	int n = send(sockfd, &s, sizeof(client_request), 0);
	DIE(n < 0, "send");

	printf("Subscribed to topic.\n");
}

void unsubscribe(int sockfd, char *topic) {
	client_request s;
	memset(&s, 0, sizeof(client_request));
	memcpy(&s.topic, topic, strlen(topic));
	s.req_type = UNSUBSCRIBE;
	s.sf = 0;

	int n = send(sockfd, &s, sizeof(client_request), 0);
	DIE(n < 0, "send");

	printf("Unsubscribed from topic.\n");
}

void print_output(sub_message response) {
	// ip si port
	cout << inet_ntoa(response.from_station.sin_addr) << ":" 
		 << ntohs(response.from_station.sin_port) << " - ";
	
	// topic
	cout << response.message.topic << " - ";

	// data_type and value
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

void handle_stdin_message(int sockfd) {
    char buffer[BUFLEN];

    memset(buffer, 0, BUFLEN);
    fgets(buffer, BUFLEN - 1, stdin);

    if (strncmp(buffer, "exit", 4) == 0) {
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(0);
    }

    char *command;
    command = strtok(buffer, " ");

    if (strncmp(command, "subscribe", 10) == 0) {
        char *topic = strtok(NULL, " ");
        if (topic == NULL) return;

        char *sf = strtok(NULL, " \n"); 
        if (sf == NULL) return;

        subscribe(sockfd, topic, sf[0]);
        return;
    }

    if (strncmp(command, "unsubscribe", 12) == 0) {
        char *topic = strtok(NULL, " \n");
        if (topic == NULL) return;

        unsubscribe(sockfd, topic);
    }
}

void handle_server_message(int sockfd) {
    sub_message response;
    memset(&response, 0, sizeof(sub_message));

    int bytes_recv = recv(sockfd, &response, sizeof(sub_message), 0);
    DIE(bytes_recv < 0, "receive");

    if (bytes_recv == 0) {
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        return;
    }

    print_output(response);
}
