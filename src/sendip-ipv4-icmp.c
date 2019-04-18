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

void *Biblioteka;


int main(int argc, char **argv) {

	Biblioteka = dlopen("/home/student/Downloads/sendip-ipv4-icmp-master/src/biblioteka.so", RTLD_NOW);
	  	if (!Biblioteka) {
	  		printf("Error otwarcia: %s\n", dlerror());
	  		return (1);
	  	}

	  	typedef void (*Glowna)();
	  	Glowna glowna = (Glowna)dlsym(Biblioteka, "glowna");
	  	glowna(argc, argv);
}
