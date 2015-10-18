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
		//tablice przechowujace rozmiary macierzy A i B
		int sizeA[2];
		int sizeB[2];		
		
		//odczytujemy z serwera rozmiary macierzy A i B
		read(sck, sizeA, 8);
		read(sck, sizeB, 8);

		//alokacja pamieci dla macierzy A i B
		float** matrixA = (float**)malloc(sizeA[0] * sizeof(float*));
                int i;
                for(i = 0; i < sizeA[0]; i++){
                       matrixA[i] = (float*)malloc(sizeA[1] * sizeof(float));
                }
		
                float** matrixB = (float**)malloc(sizeB[0] * sizeof(float*));
                for(i = 0; i < sizeB[0]; i++){
                        matrixB[i] = (float*)malloc(sizeB[1] * sizeof(float));
                }
		int j;
		//odczyt macierzy A i B
		for(i = 0; i < sizeA[0]; i++){
			for(j = 0; j < sizeA[1]; j++){
				read(sck, &matrixA[i][j], 4);
			}
		}
		for(i = 0; i < sizeB[0]; i++){
			for(j = 0; j < sizeB[1]; j++){
				read(sck, &matrixB[i][j], 4);
			}
		}
		
		//wyswietlanie macierzy A i B
		//int j;
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
		
		//alokowanie pamieci dla macierzy C
		float** matrixC = (float**)malloc(sizeA[0] * sizeof(float*));
                for(i = 0; i < sizeA[0]; i++){
                        matrixC[i] = (float*)malloc(sizeB[1] * sizeof(float));
                }

		//mnozenie macierzy A i B (A x B = C)
		int k;
		for(i = 0; i < sizeA[0]; i++){
			for(j = 0; j < sizeB[1]; j++){
				for(k = 0; k<sizeA[1]; k++){
				matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
				}
			}
		}

		//wyswietlanie macierzy wynikowej C
		printf("\nMatrixC:\n");
                for(i = 0; i < sizeA[0]; i++){
                        for(j = 0; j < sizeB[1]; j++){
                                printf("%f ",  matrixC[i][j]);
                        }
                        printf("\n");
                }


		//wysylanie macierzy wynikowej C
		for(i = 0; i<sizeA[0]; i++){
			write(sck, matrixC[i], 4 * sizeB[1]);
		}

		//dealokacja pamieci macierzy A,B i C
		free(matrixA);
		free(matrixB);
		free(matrixC);

		getchar();
		
		//zamykanie deskryptorow
		close(sck);
		
		return 0; 

} 

