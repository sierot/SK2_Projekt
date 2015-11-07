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

int main(int argc, char* argv[]){
		
//vvvvvvvvvvvvvv TWORZENIE POLACZENIA vvvvvvvvvvvvvv
  
		char* server = argv[1]; //pobranie adresu IPv4
		
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
		printf("Rozpoczynam laczenie z: \"%s\"...\n", server);
		int cnct = connect(sck, (struct sockaddr*) &addr, sizeof(struct sockaddr));
		if (cnct < 0){
			printf("ERROR (cnct): %d %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}else{
			printf("Polaczony z serwerem.\n");
		}
		//tablice przechowujace rozmiary macierzy A i B
		int sizeA[2];
		int sizeB[2];		
		
//^^^^^^^^^^^^^^ TWORZENIE POLACZENIA ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv PRZYGOTOWANIE MACIERZY vvvvvvvvvvvvvv		

		//odczytujemy z serwera rozmiary macierzy A i B
		read(sck, sizeA, 8);
		read(sck, sizeB, 8);
		printf("A = [%d,%d] B = [%d, %d]\n", sizeA[0], sizeA[1], sizeB[0], sizeB[1]);
		//alokacja pamieci dla macierzy A i B
		float** matrixA = (float**)malloc(sizeA[0] * sizeof(float*));
                int i, j;
                for(i = 0; i < sizeA[0]; i++){
                       matrixA[i] = (float*)malloc(sizeA[1] * sizeof(float));
                }
		
                float** matrixB = (float**)malloc(sizeB[0] * sizeof(float*));
                for(i = 0; i < sizeB[0]; i++){
                        matrixB[i] = (float*)malloc(sizeB[1] * sizeof(float));
                }
                
//^^^^^^^^^^^^^^ PRZYGOTOWANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv POBIERANIE MACIERZY vvvvvvvvvvvvvv	
                
		//zmienne do liczenia czasu
		double read_start, read_stop, start, stop, write_start, write_stop;
		read_start = omp_get_wtime();
		//odczyt macierzy A i B
		int pakiety = 0;
		int sum = 0;
		int sum_temp = 0;
		for(i = 0; i < sizeA[0]; i++){
			while(sum < 4*sizeA[1]){
			  sum_temp = read(sck, (matrixA[i] + sum/4), (4*sizeA[1]-sum));
			  if(sum_temp > 0){
			     //jezeli wczytalismy nie pelnego floata (4B) to musimy doczytac do 4B
			     while(sum_temp%4 != 0){
				void* v = matrixA[i] + sum/4 + sum_temp/4;
				v = v + (sum_temp%4);
				int sum_temp_temp = read(sck, v, (4 - sum_temp%4));
				sum_temp += sum_temp_temp;
			     }
			     sum += sum_temp;
			  }
			  else{
			    printf("A ERROR i = %d read = %d\n", i, sum_temp);
			    exit(EXIT_FAILURE);
			  }
			}
			pakiety += sum;
			sum = 0;
			sum_temp = 0;
		}
		printf("Odebrane pakiety macierzy A: %dB\n", pakiety);
		pakiety = 0;
		sum = 0;
		for(j = 0; j < sizeB[0]; j++){
			while(sum < 4*sizeB[1]){
			  sum_temp = read(sck, (matrixB[j] + sum/4), (4*sizeB[1]-sum));
			  if(sum_temp > 0){
			     if(sum_temp%4 != 0){
				void* v = matrixB[j] + sum/4 + sum_temp/4;
				v = v + (sum_temp%4);
				int sum_temp_temp = read(sck, v, (4 - sum_temp%4));
				sum_temp += sum_temp_temp;
			     }
			     sum += sum_temp;
			  }
			  else{
			    printf("B ERROR j = %d read = %d\n", j, sum_temp);
			    exit(EXIT_FAILURE);
			  }
			}
			pakiety += sum;
			sum = 0;
			sum_temp = 0;
		}
		printf("Odebrane pakiety macierzy B: %dB\n", pakiety);
		pakiety = 0;
		sum = 0;
		read_stop = omp_get_wtime();
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
		
//^^^^^^^^^^^^^^ POBIERANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv WYLICZANIE MACIERZY vvvvvvvvvvvvvv
		
		//alokowanie pamieci dla macierzy C
		float** matrixC = (float**)malloc(sizeA[0] * sizeof(float*));
                for(i = 0; i < sizeA[0]; i++){
                        matrixC[i] = (float*)malloc(sizeB[0] * sizeof(float));
                }
                
		start = omp_get_wtime();
		//mnozenie macierzy A i B (A x B = C)
		omp_set_num_threads(2);
		#pragma omp parallel
		{
			int k, i, j;
			#pragma omp for
			for(i = 0; i < sizeA[0]; i++){
				for(j = 0; j < sizeB[0]; j++){
					float dot = 0;
					for(k = 0; k<sizeA[1]; k++){
						dot += (matrixA[i][k] * matrixB[j][k]);
					}
					matrixC[i][j] = dot;
				}
			}
		}
		stop = omp_get_wtime();
		//wyswietlanie macierzy wynikowej C
		/*printf("\nMatrixC:\n");
                for(i = 0; i < sizeA[0]; i++){
                        for(j = 0; j < sizeB[0]; j++){
                                printf("%f ",  matrixC[i][j]);
                        }
                        printf("\n");
                }*/

//^^^^^^^^^^^^^^ WYLICZANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv WYSYLANIE MACIERZY vvvvvvvvvvvvvv
		
		write_start = omp_get_wtime();
		//wysylanie macierzy wynikowej C
		pakiety = 0;
		for(i = 0; i<sizeA[0]; i++){
			pakiety += write(sck, matrixC[i], 4* sizeB[0]);
		}
		//printf("Sum = %d\n", sum);
		printf("Wyslane pakiety macierzy C: %dB\n", pakiety);
		

//^^^^^^^^^^^^^^ WYSYLANIE MACIERZY ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv WYSWIETLANIE WYNIKOW vvvvvvvvvvvvvv
				
		
		write_stop = omp_get_wtime();
		double czas = stop-start;
		printf("Czas przetwarzania: %f sekund.\n", write_stop-read_start);
		printf("Czas wczytywania: %f sekund.\n", read_stop-read_start);
		printf("Czas mnozenia: %f sekund.\n", czas);
		printf("Czas wysylania: %f sekund.\n", write_stop-write_start);
		//wysylanie czasu mnozenia
		write(sck, &czas, sizeof(double));
		
//^^^^^^^^^^^^^^ WYSWIETLANIE WYNIKOW ^^^^^^^^^^^^^^

//vvvvvvvvvvvvvv SPRZATANIE vvvvvvvvvvvvvv

		for(i = 0; i < sizeA[0]; i++)
		  free(matrixA[i]);
		for(i = 0; i < sizeB[0]; i++)
		  free(matrixB[i]);
		for(i = 0; i < sizeA[0]; i++)
		  free(matrixC[i]);

		free(matrixA);
		free(matrixB);
		free(matrixC);		
		//czekanie na pozwolenie na zakonczenie polaczenia
		int ack = 1;
		read(sck, &ack, 4);
		while(ack != -1){
		  read(sck, &ack, 4);
		}
		close(sck);
		return 0; 

} 

