// Protocoale de comunicatii
// Laborator 9 - DNS
// dns.c

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int usage(char* name)
{
	printf("Usage:\n\t%s -n <NAME>\n\t%s -a <IP>\n", name, name);
	return 1;
}

/*
 * TODO:
 * La exercitiul 2 ne alegem o singura gazda din tabel si
 * rezolvam pentru el. Trebuie sharescreen in arhiva de lab.
 */

// Receives a name and prints IP addresses
void get_ip(char* name)
{
	int ret;
	struct addrinfo hints, *result, *p;
	char ip_addr[50];
	struct sockaddr_in  *addr_ipv4;
	struct sockaddr_in6 *addr_ipv6;

	// DONE: set hints
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags  = AI_PASSIVE | AI_CANONNAME; 

	// DONE: get addresses
	ret = getaddrinfo(name, NULL, &hints, &result);
	const char *error = gai_strerror(ret);
	printf("%s\n", error);

	// DONE: iterate through addresses and print them
	p = result;
	while(p != NULL) {
		if (p->ai_family == AF_INET) // ipv4
		{
			addr_ipv4 = (struct sockaddr_in *)p->ai_addr;
			inet_ntop(p->ai_family, &(addr_ipv4->sin_addr), ip_addr, sizeof(ip_addr));
			printf("IP is %s\n", ip_addr);
		} 
		else if (p->ai_family == AF_INET6) // ipv6
		{
			addr_ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			inet_ntop(p->ai_family, &(addr_ipv6->sin6_addr), ip_addr, sizeof(ip_addr));
			printf("IP is %s\n", ip_addr);
		}

		memset(ip_addr, 0, sizeof(ip_addr));
		p = p->ai_next;
	}

	// DONE: free allocated data
	freeaddrinfo(result);
}

// Receives an address and prints the associated name and service
void get_name(char* ip)
{
	int ret;
	struct sockaddr_in addr;
	char host[1024];
	char service[20];

	// DONE: fill in address data
	addr.sin_family = AF_INET;
	inet_aton(ip, &addr.sin_addr);
	addr.sin_port = htons(80);

	// DONE: get name and service
	getnameinfo((struct sockaddr *)(&addr), sizeof(addr), host, sizeof(host), service, sizeof(service), 0);
	
	// DONE: print name and service
	printf("Host is %s and uses sservice %s\n", host, service);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		return usage(argv[0]);
	}

	if (strncmp(argv[1], "-n", 2) == 0) {
		get_ip(argv[2]);
	} else if (strncmp(argv[1], "-a", 2) == 0) {
		get_name(argv[2]);
	} else {
		return usage(argv[0]);
	}

	return 0;
}
