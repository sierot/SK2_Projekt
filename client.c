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
#include <getopt.h>

#include "matrixLoader.h"
#include "division.h"

struct server{
	int privSizeA[2];
	int privSizeB[2];
};

//struktura potrzebna do dlugich nazw z getopt
struct option long_options[] =
{
  {"Apath",  		required_argument, NULL, 'A'},
  {"Bpath",  		required_argument, NULL, 'B'},
  {"Cpath",  		required_argument, NULL, 'C'},
  {"addresses",  	required_argument, NULL, 'a'},
  {"firstserver",    	required_argument, NULL, 'f'},
  {"numberofservers",   required_argument, NULL, 'n'},
  {"port",    		required_argument, NULL, 'p'},
  {"statistics",    	no_argument, NULL, 's'},
  {"stats",    		no_argument, NULL, 's'},
  {0, 0, 0, 0}
};

int main(int argc, char* argv[]){
		
//vvvvvvvvvvvvvv SPRAWDZANIE WARUNKU MNOZENIA MACIERZY vvvvvvvvvvvvvv
  
  		//tablice przechowujace rozmiar macierzy A i B
		int sizeA[2];
		int sizeB[2];
		
		//domysle wartosci do ustawien
		char* Apath = "Amatrix";
		char* Bpath = "Bmatrix";
		char* Cpath = "Cmatrix";
		char* AdressPath = "adresy";
		char* ServerStart = "1";
		char* ServerNum = "1";
		char* ServersPorts = "23000";
		int stats = 0;

		int gopt; //cala magia getopt
		while ((gopt = getopt_long (argc, argv, "A:B:C:a:f:n:p:s", long_options, NULL)) != -1){
		    switch (gopt){
		      case 'A':
			Apath = optarg;
			break;
		      case 'B':
			Bpath = optarg;
			break;
		      case 'C':
			Cpath = optarg;
			break;
		      case 'a':
			AdressPath = optarg;
			break;
		      case 'f':
			ServerStart = optarg;
			break;
		      case 'n':
			ServerNum = optarg;
			break;
		      case 'p':
			ServersPorts = optarg;
			break;
		      case 's':
			stats = 1;
			break;
		    }
		}	
		
		//pobieranie do talbic wielkosci macierzy z pliku txt (matrixLoader.c)
		loadSize(Apath, sizeA);
                loadSize(Bpath, sizeB);
  
		//warunek konieczny mnozenia macierzy
		if(sizeA[1] != sizeB[1]){
			printf("ERROR: Nie moge pomnozyc macierzy o takich wymiarach: [%d,%d] x [%d,%d]\n", sizeA[0], sizeA[1], sizeB[0], sizeB[1]);
			exit(EXIT_FAILURE);
		}
		
//^^^^^^^^^^^^^^ SPRAWDZANIE WARUNKU MNOZENIA MACIERZY ^^^^^^^^^^^^^^
  
//vvvvvvvvvvvvvv TWORZENIE POLACZENIA vvvvvvvvvvvvvv
		
		//tworzenie struktury potrzebnej do wykonania connect(...)
		struct sockaddr_in addr;
		addr.sin_family = AF_INET; //protokol IPv4
		addr.sin_port = htons(atoi(ServersPorts)); //port
		//addr.sin_addr.s_addr = inet_addr(server); //adres IPv4 serwera
		
		int i, c;
		int serv_num = atoi(ServerNum);
		
		int* sck_tab = (int*) malloc (serv_num * sizeof(int));
				
		FILE* input;
		input = fopen(AdressPath,"r");
		char* adr = (char*) malloc (20 * sizeof(char));
		
		for(i = 1; i < atoi(ServerStart); i++){
		   fgets(adr , 20 , input); 
		}
		
		for(i = 0; i < serv_num; i++){
			sck_tab[i] = socket(AF_INET, SOCK_STREAM, 0);
			if (sck_tab[i] < 0){
				printf("ERROR (sck): %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			
			if ( fgets(adr , 20 , input) == NULL ){		      
			  printf("Cos nie tak z plikiem adresy\n");
			}
			char* server = adr; //pobranie adresu IPv4
			if(server == NULL){
			    printf("Musisz podać IPv4 servera.\n");
			    exit(EXIT_FAILURE);
			}
			
			addr.sin_addr.s_addr = inet_addr(server); //adres IPv4 serwera
			//laczenie sie z serwerem
			int cnct = connect(sck_tab[i], (struct sockaddr*) &addr, sizeof(struct sockaddr));
			if (cnct < 0){
				printf("ERROR (cnct[%d]): %d %s\n", i, errno, strerror(errno));
				exit(EXIT_FAILURE);
			}else{
				printf("Server 1: %s", server);
			}
			
		}
		printf("\n");
		free(adr);
		fclose(input);
		
		
//^^^^^^^^^^^^^^ TWORZENIE POLACZENIA ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv PRZYGOTOWANIE MACIERZY vvvvvvvvvvvvvv
		
		double read_start, read_stop, start, stop, write_start, write_stop, t_stop;
                read_start = omp_get_wtime();

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
		loadFile(Apath, matrixA, sizeA);
		loadFile(Bpath, matrixB, sizeB);
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
		
	        //tablice przechowujace granice serverow
	        //granice serverow to wspolrzedne w macierzy mowiace jakie elementy dany procesor bedzie mial wyliczyc
		int** tabi = (int**) malloc (serv_num * sizeof(int*));
     		for(i = 0; i < serv_num; i++){
                	tabi[i] = (int*) malloc (2 * sizeof(int*));
        	}

        	int** tabj = (int**) malloc (serv_num * sizeof(int*));
	        for(i = 0; i < serv_num; i++){
        	        tabj[i] = (int*) malloc (2 * sizeof(int*));
	        }

		//wyliczamy granice serverow (division.c)
		divide(serv_num, sizeB[0], sizeA[0], tabi, tabj);
		
// 		if(stats){
// 		  for(i = 0; i < serv_num; i++)
// 		  printf("Server %d ma macierz: i:[%d,%d]   j:[%d,%d]\n", i+atoi(ServerStart), tabi[i][0], tabi[i][1], tabj[i][0], tabj[i][1]);
// 		}
		//struktura servera
                struct server* servers_tab = malloc (serv_num * sizeof(struct server));
		//(FOR) do wyliczenia nie potrzebujemy calej tablicy (wystarczy dana czesc)
		for(c = 0; c < serv_num; c++){
			servers_tab[c].privSizeA[0] = tabi[c][1] - tabi[c][0] + 1;
			servers_tab[c].privSizeB[0] = tabj[c][1] - tabj[c][0] + 1;
		}
		
		//(FOR) ale zeby wyliczyc dany element potrzebujemy obu calych wierszy
		for(c = 0; c < serv_num; c++){
			servers_tab[c].privSizeA[1] = sizeA[1];
			servers_tab[c].privSizeB[1] = sizeB[1];
		}
		
		if(stats){
		  for(c = 0; c < serv_num; c++){
			  printf("Rozmiar macierzy servera %d: A = [%d,%d] B = [%d,%d]\n", c+atoi(ServerStart),
				servers_tab[c].privSizeA[0], servers_tab[c].privSizeA[1], servers_tab[c].privSizeB[0], servers_tab[c].privSizeB[1]);
		  }
		}
		printf("\n");
//^^^^^^^^^^^^^^ PRZYGOTOWANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv WYSYLANIE MACIERZY vvvvvvvvvvvvvv
		
		//start liczenia czasu		
		start = omp_get_wtime();
		//(FOR)wysylamy serverowi rozmiary jego macierzy A i B
		for(c = 0; c < serv_num; c++){
			write(sck_tab[c], servers_tab[c].privSizeA, 8);
			write(sck_tab[c], servers_tab[c].privSizeB, 8);
		}
		int pakiety = 0;
		int sum = 0;
		//wyslanie macierzy A i B wiersz po wierszu
		for(i = 0; i < sizeA[0]; i++){
			for(c = 0; c < serv_num; c++){
				if((i >= tabi[c][0]) && (i <= tabi[c][1])){
					sum = write(sck_tab[c], matrixA[i], 4*sizeA[1]);
					pakiety += sum;
				}
			}
			
		}
		if(stats) printf("Wyslane dane macierzy A: %dB\n", pakiety);
		pakiety = 0;
		sum = 0;
		for(j = 0; j < sizeB[0]; j++){	
			for(c = 0; c < serv_num; c++){
				if((j >= tabj[c][0]) && (j <= tabj[c][1])){
                                	 sum = write(sck_tab[c], matrixB[j], 4*sizeB[1]);
					 pakiety += sum;
				}
			}
			
		}
		if (stats) printf("Wyslane dane macierzy B: %dB\n", pakiety);
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
                
		for(c = 0; c < serv_num; c++){
			for(i = tabi[c][0]; i <= tabi[c][1]; i++){	
			  
				while(sum < 4*servers_tab[c].privSizeB[0]){
				  //pobieramy kawalek wiersza (od tabj[c][0]) 
				  sum_temp = read(sck_tab[c], (matrixC[i] + tabj[c][0] + sum/4), (4*servers_tab[c].privSizeB[0]-sum));
				  //jesli calego nie wczytamy to reszte musimy doczytac pozniej w odpowiednie miejsce
				  if(sum_temp > 0){
				    //jesli wczytalismy niepelego floata (4B) to musimy doczytac do 4B
				    if(sum_temp%4 != 0){
				      void* v = matrixC[i] + tabj[c][0] + sum/4 + sum_temp/4;
				      v = v + (sum_temp%4);
				      int sum_temp_temp = read(sck_tab[c], v, (4 - sum_temp%4));
				      sum_temp += sum_temp_temp;
				    }
				    sum += sum_temp;
				  }
				  else{
				    printf("ERROR c = %d, i = %d read = %d\n", c, i, sum_temp);
				    exit(EXIT_FAILURE);
				  }
				}
				
				pakiety += sum;
				//printf("pakiety = %d\n", pakiety);
				sum_temp = 0;
				sum = 0;
			}
			
		}
                

		if(stats) printf("Pobrane dane z macierzy C: %dB\n\n", pakiety);

		//wyswietlanie macierzy wynikowej C
		/*printf("\nMatrixC:\n");
		for(i = 0; i<sizeA[0]; i++){ 
                        for(j = 0; j<sizeB[0]; j++){
                                printf("%f ",  matrixC[i][j]);
                        }
                        printf("\n");
                }*/
		write_stop = omp_get_wtime();
		
		//zapis do pliku 
		int desc = open(Cpath, O_WRONLY | O_TRUNC | O_CREAT, 0666);
		for(i = 0; i < sizeA[0]; i++){
			write(desc, matrixC[i], 4*sizeB[0]);
		}
		
//^^^^^^^^^^^^^^ POBIERANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv WYSWIETLANIE WYNIKOW vvvvvvvvvvvvvv
		
		t_stop = omp_get_wtime();
                
		
		double* time_tab = (double*) malloc (serv_num * sizeof(double));
		
		for(i = 0; i < serv_num; i++){
		    read(sck_tab[i], &time_tab[i], sizeof(double));
		}
		double time_sum = 0;
		
		FILE* plik; 
		plik = fopen("Czasy.txt", "a");
		fprintf(plik, "\n");
		//fprintf(plik, "Wielkosc instancji: [%d,%d] x [%d,%d]\n", sizeA[0], sizeA[1], sizeB[0], sizeB[1]);
		fprintf(plik, "%d;", sizeA[0]);
		//printf("Wielkosc instancji: [%d,%d] x [%d,%d]\n", sizeA[0], sizeA[1], sizeB[0], sizeB[1]);
		//fprintf(plik, "Liczba serwerow: %d\n", serv_num);
		
		
		
		
		fprintf(plik, "%f\n", t_stop-read_start);
		//fprintf(plik, "Aproksymacyjny czas sekwencyjnego przetwarzania: %f\n", WSP_PRZYS*time_sum);
		//fprintf(plik, "Aproksymacyjny czas komunikacji: %f\n", (t_stop-read_start) - time_sum/serv_num);
		if(stats){
		    printf("Wielkosc mnozonych macierzy: [%d,%d] x [%d,%d]\n\n", sizeA[0], sizeA[1], sizeB[0], sizeB[1]);
		    
		    printf("Czas wczytywania z pliku: %f sekund.\n", read_stop-read_start);
		    printf("Czas wysylania: %f sekund.\n", stop-start);
		    printf("Czas odbioru: %f sekund.\n", write_stop-write_start);
		    printf("Czas zapisu do pliku: %f sekund.\n\n", t_stop-write_stop);
		    for(i = 0; i < serv_num; i++){
		      //fprintf(plik, "Czas serwera %d: %f\n", i, time_tab[i]);
		      printf("Czas przetwarzania serwera %d: %f\n", i+atoi(ServerStart), time_tab[i]); 
		      time_sum += time_tab[i];
		    }
		    printf("\nCalkowity czas przetwarzania: %f sekund.\n", t_stop-read_start);
		    printf("Aproksymacyjny czas stracony na komunikacji: %f\n", (t_stop-read_start) - time_sum/serv_num);
		}
		
		//pozwolenie na zakonczenie polaczenia
// 		int ack = -1;
// 		for(i = 0; i < serv_num; i++)
// 		  write(sck_tab[i], &ack, 4);
// 		
		
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
		
		for(i = 0; i < serv_num; i++){
			close(sck_tab[i]);
			free(tabi[i]);
			free(tabj[i]);
		}
		
		free(tabi);
		free(tabj);
		free(sck_tab);
		free(servers_tab);
		
		return 0; 

} 
