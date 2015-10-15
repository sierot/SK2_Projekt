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
		
		//tworzenie gniazda (otrzymanie deskryptora sck)	
		int sck = socket(AF_INET, SOCK_STREAM, 0);
		if(sck < 0){
			printf("ERROR (sck): %s\n", strerror(errno));
			exit(EXIT_FAILURE);		
		}

		//tworzorzenie struktury potrzebnej do zwiazania gniazda
		struct sockaddr_in addr;
                addr.sin_family = AF_INET; //protokol IPv4
                addr.sin_port = htons(23000); //port
                addr.sin_addr.s_addr = htonl(INADDR_ANY); //adres IPv4 klienta

		//wiazanie gniazda z deskryptorem sck z wykorzystaniem struktury sockaddr_in
		int bnd = bind(sck, (struct sockaddr*) &addr, sizeof(struct sockaddr));
		if (bnd < 0){
			printf("ERROR (bnd): %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		//rozpoczecie nasluchiwania zgloszen i ustalenie dlugosci kolejki do serwera
		int lsn = listen(sck, 0);	
		if (lsn < 0){
			printf("ERROR (lsn): %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		//deklaracja struktur potrzebnych do odboru polaczenia od klienta
		struct sockaddr_in c_addr; //struktura do informacji o gniezdzie kliencie
		int nTmp = sizeof(struct sockaddr); //aby nie bylo warninga ;)

		//oczekiwanie na connect(...) ze strony klienta i odebranie deskryptora do klienta		
		printf("Oczekuje na polaczenie...\n");
		int c_sck = accept(sck, (struct sockaddr*)&c_addr, &nTmp);
		if(c_sck < 0){
                       	printf("ERROR (c_sck): %s\n", strerror(errno));
                	exit(EXIT_FAILURE);
		}
		
		//potwierdzenie polaczenia
		printf("Polaczony z: %s.\n", inet_ntoa((struct in_addr)c_addr.sin_addr));	
		
		//alokowanie pamieci dla macierzy A
		int** matrixA = (int**)malloc(4 * sizeof(int*));
		int i;
		for(i = 0; i<4; i++){
			matrixA[i] = (int*)malloc(4 * sizeof(int));
		}
		
		//alokowanie pamieci dla macierzy B
		int** matrixB = (int**)malloc(4 * sizeof(int*));
                for(i = 0; i<4; i++){
                        matrixB[i] = (int*)malloc(4 * sizeof(int));
                }
		
		//nadanie wartosci macierza A i B
		int j;
		for(i = 0; i<4; i++){
			for(j = 0; j<4; j++){
				matrixA[i][j] = 4*i + j;
				matrixB[i][j] = 4*i + j;
			}
		}

		//wyswietlanie macierzy A i B
		printf("\nMatrixA:\n");
	        for(i = 0; i<4; i++){
                        for(j = 0; j<4; j++){
                                printf("%d ",  matrixA[i][j]);
                        }
			printf("\n");
                }

		printf("\nMatrixB:\n");
		for(i = 0; i<4; i++){
                        for(j = 0; j<4; j++){
                                printf("%d ",  matrixA[i][j]);
                        }
                        printf("\n");
                }

		//wyslanie macierzy A i B wiersz po wierszu
		for(i = 0; i<4; i++){		
			write(c_sck, matrixA[i], 16);		
			write(c_sck, matrixB[i], 16);
		}

		//dealokacja pamieci macierzy A i B
		free(matrixA);
		free(matrixB);

		//alokacja pamieci dla macierzy C
		int** matrixC = (int**)malloc(4 * sizeof(int*));
                for(i = 0; i<4; i++){
                        matrixC[i] = (int*)malloc(4 * sizeof(int));
                }

		//odczyt wartosci wyliczonej macierzy wiersz po wierszu
		for(i = 0; i<4; i++){
                        read(c_sck, matrixC[i], 16);
                }
		
		//wyswietlanie macierzy wynikowej C
		printf("\nMatrixC:\n");
		for(i = 0; i<4; i++){
                        for(j = 0; j<4; j++){
                                printf("%d ",  matrixC[i][j]);
                        }
                        printf("\n");
                }

		//dealokacja pamieci macierzy C
		free(matrixC);

		getchar();

		//zamykanie deskryptorow
		close(sck);
		close(c_sck);
		return 0; 

} 

