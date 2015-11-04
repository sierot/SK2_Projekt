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
#include <omp.h>

#include "matrixLoader.h"
#include "division.h"

#define LICZBA_KLIENTOW 1
#define WSP_PRZYS 1.86
#define KLASTER 64

struct client{
	int client_id;
	int c_sck;
	int privSizeA[2];
	int privSizeB[2];
};

int main(int argc, char* argv[]){
		
//vvvvvvvvvvvvvv TWORZENIE POLACZENIA vvvvvvvvvvvvvv
  
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
		socklen_t nTmp2 = (socklen_t)nTmp;
		int i, c;
		int comp_no = LICZBA_KLIENTOW; //liczba klientow
		if(argv[1] != NULL){
		  comp_no = atoi(argv[1]);
		}
		int* sck_tab = (int*) malloc (comp_no * sizeof(int));
		for(i = 0; i < comp_no; i++){
			//oczekiwanie na connect(...) ze strony klienta i odebranie deskryptora do klienta		
			printf("Oczekuje na polaczenie...\n");
			sck_tab[i] = accept(sck, (struct sockaddr*)&c_addr, &nTmp2);
			if(sck_tab[i] < 0){
                       		printf("ERROR (c_sck[%d]): %s\n", i, strerror(errno));
                		exit(EXIT_FAILURE);
			}
			//potwierdzenie polaczenia
                        printf("Polaczony z: %s.\n", inet_ntoa((struct in_addr)c_addr.sin_addr));
		}
			
		
		//wypelnianie struktury klienta
                struct client* clients_tab = malloc (comp_no * sizeof(struct client));
		for(c = 0; c < comp_no; c++){
			clients_tab[c].c_sck = sck_tab[c];
			clients_tab[c].client_id = c;
		}

//^^^^^^^^^^^^^^ TWORZENIE POLACZENIA ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv PRZYGOTOWANIE MACIERZY vvvvvvvvvvvvvv
		
		//tablice przechowujace rozmiar macierzy A i B
		int sizeA[2];
		int sizeB[2];		
		
		//liczenie czasu trza zmienic
		double read_start, read_stop, start, stop, write_start, write_stop, t_stop;
                read_start = omp_get_wtime();
		//pobieranie do talbic wartosci z pliku txt (matrixLoader.c)
		loadSize("Amatrix", sizeA);
                loadSize("Bmatrix", sizeB);

		//warunek konieczny mnozenia macierzy
		if(sizeA[1] != sizeB[1]){
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
		read_stop = omp_get_wtime();
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
		
	        //tablice przechowujace granice klientow
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
		divide(comp_no, sizeB[0], sizeA[0], tabi, tabj);
		
		for(i = 0; i < comp_no; i++)
		printf("Klient %d ma macierz: i:[%d,%d]   j:[%d,%d]\n", i, tabi[i][0], tabi[i][1], tabj[i][0], tabj[i][1]);
	
		
		//(FOR) do wyliczenia nie potrzebujemy calej tablicy (wystarczy dana czesc)
		for(c = 0; c < comp_no; c++){
			clients_tab[c].privSizeA[0] = tabi[c][1] - tabi[c][0] + 1;
			clients_tab[c].privSizeB[0] = tabj[c][1] - tabj[c][0] + 1;
		}
		
		//(FOR) ale zeby wyliczyc dany element potrzebujemy obu calych wierszy
		for(c = 0; c < comp_no; c++){
			clients_tab[c].privSizeA[1] = sizeA[1];
			clients_tab[c].privSizeB[1] = sizeB[1];
		}
		
		for(c = 0; c < comp_no; c++){
			printf("Rozmiar macierzy klienta %d: A = [%d,%d] B = [%d,%d]\n", c,
			       clients_tab[c].privSizeA[0], clients_tab[c].privSizeA[1], clients_tab[c].privSizeB[0], clients_tab[c].privSizeB[1]);
		}

//^^^^^^^^^^^^^^ PRZYGOTOWANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv WYSYLANIE MACIERZY vvvvvvvvvvvvvv
		
		//start liczenia czasu		
		start = omp_get_wtime();
		//(FOR)wysylamy klientowi rozmiary jego macierzy A i B
		for(c = 0; c < comp_no; c++){
			write(sck_tab[c], clients_tab[c].privSizeA, 8);
			write(sck_tab[c], clients_tab[c].privSizeB, 8);
		}
		int pakiety = 0;
		int sum = 0;
		//wyslanie macierzy A i B wiersz po wierszu
		for(i = 0; i < sizeA[0]; i++){
			for(c = 0; c < comp_no; c++){
				if((i >= tabi[c][0]) && (i <= tabi[c][1])){
					sum = write(sck_tab[c], matrixA[i], 4*sizeA[1]);
					pakiety += sum;
				}
			}
			
		}
		printf("Wyslane pakiety macierzy A: %dB\n", pakiety);
		pakiety = 0;
		sum = 0;
		for(j = 0; j < sizeB[0]; j++){	
			for(c = 0; c < comp_no; c++){
				if((j >= tabj[c][0]) && (j <= tabj[c][1])){
                                	 sum = write(sck_tab[c], matrixB[j], 4*sizeB[1]);
					 pakiety += sum;
				}
			}
			
		}
		printf("Wyslane pakiety macierzy B: %dB\n", pakiety);
		stop = omp_get_wtime();
		
		//dealokacja pamieci macierzy A i B
		for(i = 0; i < sizeA[0]; i++)
		  free(matrixA[i]);
		for(i = 0; i < sizeB[0]; i++)
		  free(matrixB[i]);
		free(matrixA);
		free(matrixB);
		
//^^^^^^^^^^^^^^ WYSYLANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv POBIERANIE MACIERZY vvvvvvvvvvvvvv		
		
		//alokacja pamieci dla macierzy C
		float** matrixC = (float**)malloc(sizeA[0] * sizeof(float*));
                for(i = 0; i<sizeA[0]; i++){
                        matrixC[i] = (float*)malloc(sizeB[0] * sizeof(float));
                }
		write_start = omp_get_wtime();
		pakiety = 0;
		sum = 0;
		int sum_temp = 0;
		//odczyt wartosci wyliczonej macierzy
                
		for(c = 0; c < comp_no; c++){
			for(i = tabi[c][0]; i <= tabi[c][1]; i++){	
			  
				while(sum < 4*clients_tab[c].privSizeB[0]){
				  //pobieramy kawalek wiersza (od tabj[c][0]) 
				  sum_temp = read(sck_tab[c], (matrixC[i] + tabj[c][0] + sum/4), KLASTER);
				  //jesli calego nie wczytamy to reszte musimy doczytac pozniej w odpowiednie miejsce
				  if(sum_temp > 0){
				    if(sum_temp%4){
				      printf("\n\nHAHA NIE\n");
				      exit(EXIT_FAILURE);
				    }
				    sum += sum_temp;
				  }
				  else{
				    printf("ERROR c = %d, i = %d read = %d\n", c, i, sum_temp);
				    exit(EXIT_FAILURE);
				  }
				}
				
				pakiety += sum;
				sum_temp = 0;
				sum = 0;
			}
			
		}
                

		printf("Dane z macierzy C: %dB\n", pakiety);
		
		/*for(i = 0; i<sizeA[0]; i++){
                        for(j = 0; j<sizeB[0]; j++){
                                if(matrixC[i][j] != 0)
				  printf("%f \n",  matrixC[i][j]);
                        }
                }
		//wyswietlanie macierzy wynikowej C
		printf("\nMatrixC:\n");
		for(i = 0; i<sizeA[0]; i++){ 
                        for(j = 0; j<sizeB[0]; j++){
                                printf("%f ",  matrixC[i][j]);
                        }
                        printf("\n");
                }*/
		write_stop = omp_get_wtime();
		
		//zapis do pliku 
		int desc = open("Cmatrix", O_WRONLY | O_TRUNC | O_CREAT, 0666);
		for(i = 0; i < sizeA[0]; i++){
			write(desc, matrixC[i], 4*sizeB[0]);
		}
		
//^^^^^^^^^^^^^^ POBIERANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv WYSWIETLANIE WYNIKOW vvvvvvvvvvvvvv
		
		t_stop = omp_get_wtime();
                printf("Czas wczytywania z pliku: %f sekund.\n", read_stop-read_start);
                printf("Czas wysylania: %f sekund.\n", stop-start);
                printf("Czas odbioru: %f sekund.\n", write_stop-write_start);
		printf("Czas zapisu do pliku: %f sekund.\n", t_stop-write_stop);
		
		double* time_tab = (double*) malloc (comp_no * sizeof(double));
		
		for(i = 0; i < comp_no; i++){
		    read(sck_tab[i], &time_tab[i], sizeof(double));
		}
		double time_sum = 0;
		
		FILE* plik; 
		plik = fopen("Czasy.txt", "a");
		fprintf(plik, "\n");
		fprintf(plik, "Wielkosc instancji: [%d,%d] x [%d,%d]\n", sizeA[0], sizeA[1], sizeB[0], sizeB[1]);
		printf("Wielkosc instancji: [%d,%d] x [%d,%d]\n", sizeA[0], sizeA[1], sizeB[0], sizeB[1]);
		fprintf(plik, "Liczba klientow: %d\n", comp_no);
		printf("Liczba klientow: %d\n", comp_no);
		for(i = 0; i < comp_no; i++){
		  fprintf(plik, "Czas klienta %d: %f\n", i, time_tab[i]);
		  printf("Czas klienta %d: %f\n", i, time_tab[i]); 
		  time_sum += time_tab[i];
		}
		
		
		fprintf(plik, "Czas przetwarzania: %f sekund.\n", t_stop-read_start);
		fprintf(plik, "Aproksymacyjny czas sekwencyjnego przetwarzania: %f\n", WSP_PRZYS*time_sum);
		fprintf(plik, "Aproksymacyjny czas komunikacji: %f\n", (t_stop-read_start) - time_sum/comp_no);
		
		printf("Czas przetwarzania: %f sekund.\n", t_stop-read_start);
		printf("Aproksymacyjny czas sekwencyjnego przetwarzania: %f\n", WSP_PRZYS*time_sum);
		printf("Aproksymacyjny czas komunikacji: %f\n", (t_stop-read_start) - time_sum/comp_no);
		
		
		//pozwolenie na zakonczenie polaczenia
		int ack = -1;
		for(i = 0; i < comp_no; i++)
		  write(sck_tab[i], &ack, 4);
		
		
//^^^^^^^^^^^^^^ WYSWIETLANIE WYNIKOW ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv SPRZATANIE vvvvvvvvvvvvvv
		fclose(plik);
		//dealokacja pamieci macierzy
		for(i = 0; i < sizeA[0]; i++){
		  free(matrixC[i]);
		}
		free(matrixC);
		free(time_tab);
		//zamykanie deskryptorow
		close(desc);
		
		/*printf("TERAZ\n");
		int jeden = 1;
		int dwa = 2;
		sleep(3);
		write(sck_tab[0], &jeden, 4);
		printf("Wyslalem\n");
		int re = read(sck_tab[0], &dwa, 4);
		printf("dwa = %c, re = %d\n", dwa, re);	
		
		printf("KONIEC\n");
		*/
		
		
		for(i = 0; i < comp_no; i++){
			close(sck_tab[i]);
			free(tabi[i]);
			free(tabj[i]);
		}
		
		free(tabi);
		free(tabj);
		free(sck_tab);
		free(clients_tab);
		
		
		
		
		close(sck);
		return 0; 

} 
