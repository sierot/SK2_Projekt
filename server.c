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
#include <time.h>

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
		
		int nFoo = 1; //dowiedz sie czo to sa za czary
		setsockopt(sck, SOL_SOCKET, SO_REUSEADDR, (char*)&nFoo, sizeof(nFoo));


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
		
		int i;
		int comp_no = 1; //liczba klientow
		int* sck_tab = (int*) malloc (comp_no * sizeof(int));
		for(i = 0; i < comp_no; i++){
			//oczekiwanie na connect(...) ze strony klienta i odebranie deskryptora do klienta		
			printf("Oczekuje na polaczenie...\n");
			sck_tab[i] = accept(sck, (struct sockaddr*)&c_addr, &nTmp);
			if(sck_tab[i] < 0){
                       		printf("ERROR (c_sck[%d]): %s\n", i, strerror(errno));
                		exit(EXIT_FAILURE);
			}
			//potwierdzenie polaczenia
                        printf("Polaczony z: %s.\n", inet_ntoa((struct in_addr)c_addr.sin_addr));
		}
			
		//PROTOTYP
		int c;
                struct client* clients_tab = malloc (comp_no * sizeof(struct client));
		for(c = 0; c < comp_no; c++){
			clients_tab[c].c_sck = sck_tab[c];
			clients_tab[c].client_id = c;
		}

		//tablice przechowujace rozmiar macierzy A i B
		int sizeA[2];
		int sizeB[2];		
		
		clock_t read_start, read_stop, start, stop, write_start, write_stop, t_stop;
                read_start = clock();
		//pobieranie do talbic wartosci z pliku txt (matrixLoader.c)
		loadSize("Amatrix", sizeA);
                loadSize("Bmatrix", sizeB);

		//warunek konieczny mnozenia macierzy
		if(sizeA[1] != sizeB[0]){
			printf("ERROR: Nie moge pomnozyc macierzy o takich wymiarach %d, %d", sizeA[1], sizeB[0]);
        	        close(sck);
	                close(clients_tab[0].c_sck);
			exit(EXIT_FAILURE);
		}

		//alokowanie pamieci dla macierzy A
		float** matrixA = (float**)malloc(sizeA[0] * sizeof(float*));
		for(i = 0; i<sizeA[0]; i++){
			matrixA[i] = (float*)malloc(sizeA[1] * sizeof(float));
		}
		
		//alokowanie pamieci dla macierzy B
		float** matrixB = (float**)malloc(sizeB[0] * sizeof(float*));
                for(i = 0; i<sizeB[0]; i++){
                        matrixB[i] = (float*)malloc(sizeB[1] * sizeof(float));
                }

		//wczytanie macierzy z pliku txt (matrixLoader.c)
		loadFile("Amatrix", matrixA, sizeA);
		loadFile("Bmatrix", matrixB, sizeB);
		read_stop = clock();
		int j;
		//wyswietlanie macierzy A i B
		/*printf("\nMatrixA:\n");
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
                }*/
		
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
                	printf("Klient %d ma przedzial: i:[%d,%d]   j:[%d,%d]\n", i, tabi[i][0], tabi[i][1], tabj[i][0], tabj[i][1]);
        	}
		
		//(FOR) do wyliczenia nie potrzebujemy calej tablicy (wystarczy czesc)
		for(c = 0; c < comp_no; c++){
			clients_tab[c].privSizeA[0] = tabi[c][1] - tabi[c][0] + 1;
			clients_tab[c].privSizeB[0] = tabj[c][1] - tabj[c][0] + 1;
		}
		
		//(FOR) ale zeby wyliczyc dany element potrzebujemy obu calych wierszy
		for(c = 0; c < comp_no; c++){
			clients_tab[c].privSizeA[1] = sizeA[1];
			clients_tab[c].privSizeB[1] = sizeB[0];
		}
		
		for(c = 0; c < comp_no; c++){
			printf("clients_tab[%d].privSizeA: [%d,%d]\n", c, clients_tab[c].privSizeA[0], clients_tab[c].privSizeA[1]);
			printf("clients_tab[%d].privSizeB: [%d,%d]\n", c, clients_tab[c].privSizeB[0], clients_tab[c].privSizeB[1]);
		}

		//start liczenia czasu		
		start = clock();
		//(FOR)wysylamy klientowi rozmiary jego macierzy A i B
		for(c = 0; c < comp_no; c++){
			write(clients_tab[c].c_sck, clients_tab[c].privSizeA, 8);
			write(clients_tab[c].c_sck, clients_tab[c].privSizeB, 8);
		}
		
		//wyslanie macierzy A i B 
		for(i = 0; i < sizeA[0]; i++){
			for(c = 0; c < comp_no; c++){
				if((i >= tabi[clients_tab[c].client_id][0]) && (i <= tabi[clients_tab[c].client_id][1]))
					write(clients_tab[c].c_sck, matrixA[i], 4*sizeA[1]);
			}
			
		}
		for(j = 0; j < sizeB[0]; j++){	
			for(c = 0; c < comp_no; c++){
				if((j >= tabj[clients_tab[c].client_id][0]) && (j <= tabj[clients_tab[c].client_id][1]))
                                	 write(clients_tab[c].c_sck, matrixB[j], 4*sizeB[1]);
			}
			
		}

		//dealokacja pamieci macierzy A i B
		free(matrixA);
		free(matrixB);
		stop = clock();
		//alokacja pamieci dla macierzy C
		float** matrixC = (float**)malloc(sizeA[0] * sizeof(float*));
                for(i = 0; i<sizeA[0]; i++){
                        matrixC[i] = (float*)malloc(sizeB[0] * sizeof(float));
                }
		write_start = clock();
		//odczyt wartosci wyliczonej macierzy
                for(i = 0; i < sizeA[0]; i++){
					for(c = 0; c < comp_no; c++){
                                        	if((i >= tabi[clients_tab[c].client_id][0]) && (i <= tabi[clients_tab[c].client_id][1])){
							read(clients_tab[c].c_sck, matrixC[i]+tabj[c][0], 4*clients_tab[c].privSizeB[0]);
						}
					}
                }

		//wyswietlanie macierzy wynikowej C
		/*printf("\nMatrixC:\n");
		for(i = 0; i<sizeA[0]; i++){
                        for(j = 0; j<sizeB[0]; j++){
                                printf("%f ",  matrixC[i][j]);
                        }
                        printf("\n");
                }*/
		write_stop = clock();
		
		/*char buf[20];
		char enter = '\n';
		char przecinek = ',';
		//zapis macierzy wynikowej C
                
		int desc = open("Cmatrix", O_WRONLY);
                for(i = 0; i<sizeA[0]; i++){
                        for(j = 0; j<sizeB[1]; j++){
                                snprintf (buf, sizeof(buf), "%f", matrixC[i][j]);
				write(desc, buf, 9);
				memset (buf, 0, sizeof(buf));
				if(j != sizeB[1] - 1){
					write(desc, &przecinek, 1);
				}

                        }
                        write(desc, &enter, 1);
                }*/
		int desc = open("Cmatrix", O_WRONLY | O_TRUNC | O_CREAT, 777);
		for(i = 0; i < sizeA[0]; i++){
			write(desc, matrixC[i], 4*sizeB[0]);
		}
		t_stop = clock();
		printf("Czas przetwarzania: %f sekund.\n", ((double)(t_stop-read_start)/1000000.0));
                printf("Czas wczytywania z pliku: %f sekund.\n", ((double)(read_stop-read_start)/1000000.0));
                printf("Czas wysylania: %f sekund.\n", ((double)(stop-start)/1000000.0));
                printf("Czas odbioru: %f sekund.\n", ((double)(write_stop-write_start)/1000000.0));
		printf("Czas zapisu do pliku: %f sekund.\n",((double)(t_stop-write_stop)/1000000.0));
		//dealokacja pamieci macierzy C
		free(matrixC);
		close(desc);
		//getchar();

		//zamykanie deskryptorow
		close(sck);
		for(i = 0; i < comp_no; i++)
			close(sck_tab[i]);
		return 0; 

} 
