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

#include "matrixLoader.h"

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
		

		//tablice przechowujace rozmiar macierzy A i B
		int sizeA[2];
		int sizeB[2];		
		
		//pobieranie do talbic wartosci z pliku txt (matrixLoader.c)
		loadSize("Amatrix.txt", sizeA);
                loadSize("Bmatrix.txt", sizeB);

		//warunek konieczny mnozenia macierzy
		if(sizeA[1] != sizeB[0]){
			printf("ERROR: Nie moge pomnozyc macierzy o takich wymiarach");
        	        close(sck);
	                close(c_sck);
			exit(EXIT_FAILURE);
		}

		//alokowanie pamieci dla macierzy A
		float** matrixA = (float**)malloc(sizeA[0] * sizeof(float*));
		int i;
		for(i = 0; i<sizeA[0]; i++){
			matrixA[i] = (float*)malloc(sizeA[1] * sizeof(float));
		}
		
		//alokowanie pamieci dla macierzy B
		float** matrixB = (float**)malloc(sizeB[0] * sizeof(float*));
                for(i = 0; i<sizeB[0]; i++){
                        matrixB[i] = (float*)malloc(sizeB[1] * sizeof(float));
                }

		//wczytanie macierzy z pliku txt (matrixLoader.c)
		loadFile("Amatrix.txt", matrixA, sizeA);
		loadFile("Bmatrix.txt", matrixB, sizeB);
		
		int j;
		//wyswietlanie macierzy A i B
		printf("\nMatrixA:\n");
	        for(i = 0; i<sizeA[0]; i++){
                        for(j = 0; j<sizeA[1]; j++){
                                printf("%f ",  matrixA[i][j]);
                        }
			printf("\n");
                }

		printf("\nMatrixB:\n");
		for(i = 0; i<sizeB[0]; i++){
                        for(j = 0; j<sizeB[1]; j++){
                                printf("%f ",  matrixB[i][j]);
                        }
                        printf("\n");
                }
		
		//wysylamy klientowi rozmiary macierz A i B
		write(c_sck, sizeA, 8);
		write(c_sck, sizeB, 8);
		
		//wyslanie macierzy A i B wiersz po wierszu
		for(i = 0; i < sizeA[0]; i++){		
			write(c_sck, matrixA[i], 4 * sizeA[1]);
		}
		for(i = 0; i < sizeB[0]; i++){	
			write(c_sck, matrixB[i], 4 * sizeB[1]);
		}

		//dealokacja pamieci macierzy A i B
		free(matrixA);
		free(matrixB);

		//alokacja pamieci dla macierzy C
		float** matrixC = (float**)malloc(sizeA[0] * sizeof(float*));
                for(i = 0; i<sizeA[0]; i++){
                        matrixC[i] = (float*)malloc(sizeB[1] * sizeof(float));
                }

		//odczyt wartosci wyliczonej macierzy wiersz po wierszu
		for(i = 0; i<sizeA[0] ; i++){
                        read(c_sck, matrixC[i], 4 * sizeB[1]);
                }
		
		//wyswietlanie macierzy wynikowej C
		printf("\nMatrixC:\n");
		for(i = 0; i<sizeA[0]; i++){
                        for(j = 0; j<sizeB[1]; j++){
                                printf("%f ",  matrixC[i][j]);
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

