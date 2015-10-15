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
		
		//alokacja pamieci dla macierzy A i B
		int** matrixA = (int**)malloc(4 * sizeof(int*));
                int i;
                for(i = 0; i<4; i++){
                       matrixA[i] = (int*)malloc(4 * sizeof(int));
                }
		
                int** matrixB = (int**)malloc(4 * sizeof(int*));
                for(i = 0; i<4; i++){
                        matrixB[i] = (int*)malloc(4 * sizeof(int));
                }

		//odczyt macierzy A i B
		for(i = 0; i<4; i++){
			read(sck, matrixA[i], 16);
			read(sck, matrixB[i], 16);
		}
		
		//wyswietlanie macierzy A i B
		int j;
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
                                printf("%d ",  matrixB[i][j]);
                        }
                      	printf("\n");
                }
		
		//alokowanie pamieci dla macierzy C
		int** matrixC = (int**)malloc(4 * sizeof(int*));
                for(i = 0; i<4; i++){
                        matrixC[i] = (int*)malloc(4 * sizeof(int));
                }

		//mnozenie macierzy A i B (A x B = C)
		int k;
		for(i = 0; i<4; i++){
			for(j = 0; j<4; j++){
				for(k = 0; k<4; k++){
				matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
				}
			}
		}

		//wyswietlanie macierzy wynikowej C
		printf("\nMatrixC:\n");
                for(i = 0; i<4; i++){
                        for(j = 0; j<4; j++){
                                printf("%d ",  matrixC[i][j]);
                        }
                        printf("\n");
                }


		//wysylanie macierzy wynikowej C
		for(i = 0; i<4; i++){
			write(sck, matrixC[i], 16);
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

