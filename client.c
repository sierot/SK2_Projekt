#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char* argv[]){
		
		char* server = argv[1]; //pobranie adresu IPv4
		printf("Rozpoczynam laczenie z: \"%s\"...\n", server);
		
		//tworzenie gniazda (otrzymanie deskryptora sck)
                int sck = socket(AF_INET, SOCK_STREAM, 0);
                if (sck < 0){
                        printf("ERROR (sck): %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                }

		//tworzenie struktury potrzebnej do wykonania connect(...)
		struct sockaddr_in addr;
		addr.sin_family = AF_INET; //protokol IPv4
		addr.sin_port = htons(23000); //port
		addr.sin_addr.s_addr = inet_addr(server); //adres IPv4 serwera
		
		//laczenie sie z serwerem
		int cnct = connect(sck, (struct sockaddr*) &addr, sizeof(struct sockaddr));
		if (cnct < 0){
			printf("ERROR (cnct): %d %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}else{
			printf("Polaczony z serwerem.\n");
		}
		
		getchar();
		close(sck);
		
		return 0; 

} 

