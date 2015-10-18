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
#include "division.h"

struct client{
	int client_id;
	int c_sck;
	int privSizeA[2];
	int privSizeB[2];
};

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
		
		//PROTOTYP
                int comp_no = 1; //liczba klientow (tymczasowe)
		struct client* clients_tab = malloc (comp_no * sizeof(struct client));
		clients_tab[0].c_sck = c_sck;
		clients_tab[0].client_id = 0;

		//tablice przechowujace rozmiar macierzy A i B
		int sizeA[2];
		int sizeB[2];		
		
		//pobieranie do talbic wartosci z pliku txt (matrixLoader.c)
		loadSize("Amatrix.txt", sizeA);
                loadSize("Bmatrix.txt", sizeB);

		//warunek konieczny mnozenia macierzy
		if(sizeA[1] != sizeB[0]){
			printf("ERROR: Nie moge pomnozyc macierzy o takich wymiarach %d, %d", sizeA[1], sizeB[0]);
        	        close(sck);
	                close(clients_tab[0].c_sck);
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
		
	        //tablice przechowujace granice procesorow
	        //granice klientow to wspolrzedne w macierzy mowiace jakie elementy dany procesor bedzie mial wyliczyc
		int** tabi = (int**) malloc (comp_no * sizeof(int*));
     		for(i = 0; i < comp_no; i++){
                	tabi[i] = (int*) malloc (2 * sizeof(int*));
        	}

        	int** tabj = (int**) malloc (comp_no * sizeof(int*));
	        for(i = 0; i < comp_no; i++){
        	        tabj[i] = (int*) malloc (2 * sizeof(int*));
	        }

		//wyliczamy granice klientow (division.c)
		divide(comp_no, sizeB[1], sizeA[0], tabi, tabj);
		
		 for(i = 0; i < comp_no; i++){
                	printf("Procesor %d ma przedzial: i:[%d,%d]   j:[%d,%d]\n", i, tabi[i][0], tabi[i][1], tabj[i][0], tabj[i][1]);
        	}

		//prywatne rozmiary macierzy A i B dla kazdego klienta
		//int client_id = 0; //kazdy klient ma swoje id (0, 1, 2...)
		//int privSizeA[2];
		//int privSizeB[2];
		
		//(FOR) do wyliczenia nie potrzebujemy calej tablicy (wystarczy czesc)
		clients_tab[0].privSizeA[0] = tabi[clients_tab[0].client_id][1] - tabi[clients_tab[0].client_id][0] + 1;
		clients_tab[0].privSizeB[1] = tabj[clients_tab[0].client_id][1] - tabj[clients_tab[0].client_id][0] + 1;

		//(FOR) ale zeby wyliczyc dany element potrzebujemy obu calych wierszy
		clients_tab[0].privSizeA[1] = sizeA[1];
		clients_tab[0].privSizeB[0] = sizeB[0];
		
		printf("clients_tab[0].privSizeA: [%d,%d]\n", clients_tab[0].privSizeA[0], clients_tab[0].privSizeA[1]);
		printf("clients_tab[0].privSizeB: [%d,%d]\n", clients_tab[0].privSizeB[0], clients_tab[0].privSizeB[1]);

		//(FOR)wysylamy klientowi rozmiary jego macierzy A i B
		write(clients_tab[0].c_sck, clients_tab[0].privSizeA, 8);
		write(clients_tab[0].c_sck, clients_tab[0].privSizeB, 8);
		
		//int k;
		//wyslanie macierzy A i B element po elemencie
		for(i = 0; i < sizeA[0]; i++){
			for(j = 0; j < sizeA[1]; j++){
					if((i >= tabi[clients_tab[0].client_id][0]) && (i <= tabi[clients_tab[0].client_id][1]))
						write(clients_tab[0].c_sck, &matrixA[i][j], 4);
			}
		}
		for(i = 0; i < sizeB[0]; i++){	
			for(j = 0; j < sizeB[1]; j++){
				 if((j >= tabj[clients_tab[0].client_id][0]) && (j <= tabj[clients_tab[0].client_id][1]))
                                	 write(clients_tab[0].c_sck, &matrixB[i][j], 4);
			}
		}

		//dealokacja pamieci macierzy A i B
		free(matrixA);
		free(matrixB);

		//alokacja pamieci dla macierzy C
		float** matrixC = (float**)malloc(sizeA[0] * sizeof(float*));
                for(i = 0; i<sizeA[0]; i++){
                        matrixC[i] = (float*)malloc(sizeB[1] * sizeof(float));
                }
		
		//odczyt wartosci wyliczonej macierzy element po elemencie
                for(i = 0; i < sizeA[0]; i++){
                        for(j = 0; j < sizeB[1]; j++){
                                        if((i >= tabi[clients_tab[0].client_id][0]) && (i <= tabi[clients_tab[0].client_id][1]) && (j >= tabj[clients_tab[0].client_id][0]) && (j <= tabj[clients_tab[0].client_id][1]))
                                                read(clients_tab[0].c_sck, &matrixC[i][j], 4);
                        }
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

