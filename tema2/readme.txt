Turcu Arina Emanuea 323CA                                                       
===================================== TEMA 2 =====================================

	Am implementat urmatoarele structuri:
- tcp_client: pastreaza ID-ul si socketul unui client TCP;
- udp_message: pastreaza un mesaj primt de la un client UDP si contine topic-ul,
	tipul de date si continutul;
- subscription: pastreaza un client si SF-ul abonamentului la un anumit topic.
	Este nevoie de ea in map-ul topics_subs (despre care am scris mai jos),
	astfel fiecarui topic ii este asociata o lista de subscription;
- client_request: reprezinta mesajul trimis de un client TCP in care cere 
	abonarea sau dezabonarea de la un topic;
- sub_message: reprezinta mesajul trimis de la server la un client TCP si 
	contine date despre clientul UDP de la care serverul a primit mesajul
	(in campul from_station) si mesajul in sine care este de tipul
	udp_message

	Implementarea este impartita in 4 fisiere sursa:
	
- server.cpp: intializeaza procesul de comunicare care tine de server (deschide
socket-ul pentru conexiuni TCP si cel pentru UDP, face bind, listen etc.), 
creeaza map-urile necesare si apoi multiplexeaza pentru a putea procesa in timp 
real mesajele venite pe orice socket. Apeleaza functiile implementate in 
server_helper.cpp pentru a procesa si a trimite mesaje. Map-urile de care m-am
folosit sunt:
	- active_clients: pastreaza toti clientii activi la un moment dat;
	- all_clients: pastreaza toti clientii care s-au conectat chiar daca
	s-au deconectat mai tarziu;
	- active_ids_sockets: pastreaza toti socketii activi la un moment dat
	impreuna cu ID-ul clientului cu care se comunica pe el;
	- topics_subs: pentru fiecare topic, pastreaza o lista cu abonamentele
	la el;
	- store_and_forward: atunci cand este nevoie sa se pastreze mesaje
	pentru a fi trimise unui client care este deconectat, se adauga in acest
	map ID-ul clientului si lista cu mesaje de trimis;
	
- server_helper.cpp: se ocupa de primirea, procesarea si trimiterea de mesaje din 
partea serverului. 
	- handle_connection_request() accepta sau refuza conexiuni cu clientii
	TCP;
	- handle_stdin_message() opreste serverul daca primeste comanda "exit";
	- handle_udp_message() primeste mesaj de la un client UDP si il trimite
	mai departe acelor clienti TCP care sunt abonati la topic-ul mesajului;
	- handle_tcp_message() primeste un mesaj de la un client TCP si ii
	anunta deconectarea, il aboneaza sau il dezaboneaza in functie de mesaj.
	
- subscriber.cpp: deschide un socket, incearca sa se conecteze la un server
folosindu-se de adresa IP si portul primite ca parametri in linia de comanda si
foloseste functiile implementate in subscriber_helper.cpp pentru a procesa si a
trimite mesaje.

- subscriber_helper.cpp: implementeaza functiile de care se foloseste server.cpp:
	- subscribe() trimite cererea de abonare la un topic catre server;
	- unsubscribe() trimite cererea de dezabonare de la un topic catre server;
	- print_output() afiseaza datele primite in mesajul primit de la server.
	- handle_stdin_message() in functie de mesajul primit de la stdin, inchide
	socket-ul asociat clientului sau apeleaza subscribe() sau unsubscribe().
	- handle_server_message() primeste mesajul de la stdin, valideaza valoarea
	intoarsa de recv() si apoi apeleaza print_output()
	

Mentiuni:	
	O parte din codul care se ocupa de initializarea serverului si a 
	subscriberului a fost preluata din scheletele din laboratoarele despre 
	TCP si UDP.
	Fisierul helpers.h este luat din laborator.
	
