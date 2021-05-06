/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	mini-server de backup fisiere
*/

#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "helpers.h"


void usage(char*file)
{
	fprintf(stderr,"Usage: %s server_port file\n",file);
	exit(0);
}

/*
*	Utilizare: ./server server_port nume_fisier
*/
int main(int argc,char**argv)
{
	int fd;

	if (argc!=3)
		usage(argv[0]);
	
	struct sockaddr_in my_sockaddr, from_station ;
	char buf[BUFLEN];


	/*Deschidere socket*/
	int sockid = socket(PF_INET, SOCK_DGRAM, 0);
	DIE(sockid == -1, "Open socket");
	
	/*Setare struct sockaddr_in pentru a asculta pe portul respectiv */
	from_station.sin_family = AF_INET;
	from_station.sin_port   = htons(atoi(argv[1]));
	from_station.sin_addr.s_addr = INADDR_ANY;
	
	int szfrmst = sizeof(from_station);

	/* Legare proprietati de socket */
	int rs = bind(sockid, (struct sockaddr *) (&from_station), sizeof(from_station));
	DIE(rs < 0, "Open bind");
	
	/* Deschidere fisier pentru scriere */
	DIE((fd=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644))==-1,"open file");
	
	/*
	*  cat_timp  mai_pot_citi
	*		citeste din socket
	*		pune in fisier
	*/
	while (1) {
		int rf = recvfrom(sockid, buf, BUFLEN, 0, (struct sockaddr *) (&from_station), &szfrmst);
		if (rf < 0) break;

		write(fd, buf, rf);
	}
	
	/*Inchidere socket*/	
	close(sockid);
	
	/*Inchidere fisier*/
	close(fd);

	return 0;
}
