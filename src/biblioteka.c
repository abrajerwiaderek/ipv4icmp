#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <unistd.h>
#include <dlfcn.h>
#include <inttypes.h>
#include "biblioteka.h"

typedef unsigned char u8;
typedef unsigned short int u16;
unsigned short chsum(unsigned short *ptr, int nbytes);
int glowna(int argc, char **argv);


unsigned short chsum(unsigned short *ptr, int nbytes) {
	register long sum;
	u_short oddbyte;
	register u_short answer;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}

	if (nbytes == 1) {
		oddbyte = 0;
		*((u_char *) &oddbyte) = *(u_char *) ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	return (answer);
}


int glowna(int argc, char **argv) {

	//Jeżeli nie podane zostaną żadne parametry zamknij program
	if (argc < 3) {
		printf(
				"\nAby wysłać pakiet uzupełnij parametry: <źródłowy adres IP> <docelowy adres IP> <interfejs> <c>\n");
		exit(0);
	}

	unsigned long daddr = inet_addr(argv[2]);
	unsigned long saddr = inet_addr(argv[1]);
	int on = 1;

	int payload_size = 0, sent, sent_size, sent_total = 1000000;

	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);


	if (sockfd < 0) {
		perror("Problem z otwarciem socketu");
		return (0);
	}




	//Dołączenie nagłowka IP
	if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (const char*) &on,
			sizeof(on)) == -1) {
		perror("Błąd generowania nagłówka ip");
		return (0);
	}

	//Zdefiniowanie wielkosci pakietu
	int packet_size = sizeof(struct iphdr) + sizeof(struct icmphdr)
			+ payload_size;
	char *packet = (char *) malloc(packet_size);

	if (!packet) {
		perror("Problem z alokowaniem pamięci");
		close(sockfd);
		return (0);
	}

	//Struktura do interfejsu
	struct ifreq ifr;

				memset(&ifr, 0, sizeof(ifr));
				snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), argv[3]);
				if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
					perror("Problem z wybraniem interfejsu");
				}

	//Struktura dotycząca nagłówka IP i ICMP
	struct iphdr *ip = (struct iphdr *) packet;
	struct icmphdr *icmp = (struct icmphdr *) (packet + sizeof(struct iphdr));

	//Uzupełnianie pamięci
	memset(packet, 0, packet_size);

	//Jeżeli czwarty parametr nie zostanie podany program wysyła pakiety z nastepujacymi parametrami
	if (argc < 5) {
		ip->tos = 0;
		ip->frag_off = 0;
		ip->ttl = 128;
		icmp->type = ICMP_ECHO; //Echo request
		icmp->code = 0;
		icmp->un.echo.id = rand();

		//Jeżeli zostanie podane -a w czwartym parametrze program odpytuje o konkretne dane
	} else if (strcmp(argv[4], "c") == 0) {
		printf("\nIle pakietow chcesz wyslac?: ");
		scanf("%d", &sent_total);
		printf("Podaj parametr Type of Service: ");
		scanf("%" SCNu8, &ip->tos);
		printf("Podaj przesunięcie: ");
		scanf("%" SCNu16, &ip->frag_off);
		printf("Podaj czas życia pakietu: ");
		scanf("%" SCNu8, &ip->ttl);
		printf("Podaj typ pakietu ICMP: ");
		scanf("%" SCNu8, &icmp->type);
		printf("Podaj kod pakietu ICMP: ");
		scanf("%" SCNu8, &icmp->code);
		printf("Podaj identyfikator pakietu ICMP: ");
		scanf("%" SCNu16, &icmp->un.echo.id);
	}

	ip->version = 4;					//Ustawienie wersji protokołu IP
	ip->id = rand();					//Identyfikator zostaje przydzielony przez funkcję rand
	ip->protocol = IPPROTO_ICMP;		//Ustawienie protokołu ICMP
	ip->saddr = saddr;					//Ustawienie adresu źródłowego
	ip->daddr = daddr;					//Ustawienie adresu docelowego
	ip->tot_len = htons(packet_size);	//Całkowita długość pakietu
	ip->ihl = 5;						//Ustawienie długości nagłówka
	ip->check = chsum((u16 *) ip, sizeof(struct iphdr));	//Suma kontrolna nagłówka danych IP
	icmp->un.echo.sequence = rand();
	icmp->checksum = 0;

	//Struktura sockaddr_in adres i port transportowy
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = saddr;
	servaddr.sin_addr.s_addr = daddr;
	memset(&servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));

	printf("\n\tRozpoczęto wysyłanie pakietów\n");

	while (sent < sent_total) {
		memset(packet + sizeof(struct iphdr) + sizeof(struct icmphdr),
				rand() % 255, payload_size);

		//Wyliczanie sumy kontorlnej nagłówka ICMP
		icmp->checksum = 0;
		icmp->checksum = chsum((unsigned short *) icmp,
				sizeof(struct icmphdr) + payload_size);

		if ((sent_size = sendto(sockfd, packet, packet_size, 0,
				(struct sockaddr*) &servaddr, sizeof(servaddr))) < 1) {
			perror("Wystąpił błąd wysyłania\n");
			break;
		}
		++sent;
		printf("\t%d pakietów wysłano\r", sent);
		fflush(stdout);

		usleep(100000);
	}

	printf("\tWYSŁANO %d PAKIET(Y/ÓW)\n\n", sent_total);

	//Zwolnienie pamięci
	free(packet);
	close(sockfd);

	return (0);
}


