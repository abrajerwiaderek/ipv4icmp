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
#include "biblioteka.h"

typedef unsigned char u8;
typedef unsigned short int u16;
void *Biblioteka; // wskaznik do bilbioteki


int main(void) {

	Biblioteka = dlopen("/home/student/Downloads/sendip-ipv4-icmp-master/src/biblioteka.so", RTLD_NOW);
	  	if (!Biblioteka) {
	  		printf("Error otwarcia: %s\n", dlerror());
	  		return (1);
	  	}

	  	typedef void (*Funkcja1)();
	  	Funkcja1 printAuthor = (Funkcja1)dlsym(Biblioteka, "printAuthor");
	  	printAuthor();
	  	typedef void (*Funkcja2)();
	  	Funkcja2 glowna = (Funkcja2)dlsym(Biblioteka, "glowna");
	  	glowna();
}
