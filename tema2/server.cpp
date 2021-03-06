#include "common_stuff.h"
#include "server_helper.h"

void usage(char *file) {
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		usage(argv[0]);
	}

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	struct sockaddr_in from_station;
	char buffer[BUFLEN];

	fd_set read_fds;
	FD_ZERO(&read_fds);

	// Deschidere socketi
	int sock_tcp = socket(PF_INET, SOCK_STREAM, 0); // TCP
	DIE(sock_tcp == -1, "Open TCP socket");
	int sock_udp = socket(PF_INET, SOCK_DGRAM, 0);
	DIE(sock_udp == -1, "Open UDP socket");

	// dezactivare algorit Nagle
	int on = 1;
	setsockopt(sock_tcp, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

	// setare struct sockaddr_in pentru a asculta pe portul respectiv
	from_station.sin_family = AF_INET;
	from_station.sin_port = htons(atoi(argv[1]));
	from_station.sin_addr.s_addr = INADDR_ANY;

	// bind
	int rs_tcp = bind(sock_tcp, (struct sockaddr *)(&from_station), sizeof(from_station));
	DIE(rs_tcp < 0, "Open bind");
	int rs_udp = bind(sock_udp, (struct sockaddr *)(&from_station), sizeof(from_station));
	DIE(rs_udp < 0, "Open bind");

	// listen
	int ret_tcp = listen(sock_tcp, MAX_CLIENTS);
	DIE(ret_tcp < 0, "listen");

	// key: socked, value: client
	unordered_map<int, tcp_client> active_clients;
	// key: client ID, client
	unordered_map<string, tcp_client> all_clients;
	// key: client ID, socket conexiune
	unordered_map<string, int> active_ids_sockets;
	// key: topic, value: lista de abonamente
	unordered_map<string, list<subscription>> topics_subs;
	// key: client ID, value: mesaje care trebuie trimise cand se conecteaza
	unordered_map<string, list<sub_message>> store_and_forward;

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sock_tcp, &read_fds);
	FD_SET(sock_udp, &read_fds);

	int fd_max = max(sock_tcp, sock_udp);

	while (1) {
		fd_set tmp_fds = read_fds;
		int ret = select(fd_max + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (int i = 0; i <= fd_max; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == 0) {
					handle_stdin_message(buffer, sock_tcp, fd_max, read_fds);
					continue;
				}

				if (i == sock_tcp) {
					int on = 1;
					setsockopt(i, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
					
					handle_connection_request(sock_tcp, fd_max, read_fds,
						active_clients, all_clients, active_ids_sockets,
						store_and_forward);
					continue;
				}

				if (i == sock_udp) {
					handle_udp_message(buffer, sock_udp, i, from_station,
						read_fds, active_ids_sockets, topics_subs,
						store_and_forward);
					continue;
				}
				
				handle_tcp_message(i, read_fds, active_clients, all_clients, 
					active_ids_sockets, topics_subs);
			}
		}
	}

	// inchidere socketi
	for (int i = 1; i <= fd_max; ++i) {
		if (FD_ISSET(i, &read_fds)) {
			shutdown(sock_tcp, SHUT_RDWR);
			close(sock_tcp);
		}
	}

	shutdown(sock_tcp, SHUT_RDWR);
	close(sock_tcp);

	return 0;
}
