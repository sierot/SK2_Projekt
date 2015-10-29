#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int main(){

	float** matrixA = (float**)malloc(4 * sizeof(float*));
        int i;
        for(i = 0; i<4; i++){
        	matrixA[i] = (float*)malloc(4 * sizeof(float));
        }


	int descA = open("Amatrix.txt", O_RDONLY);
        if(descA < 0){
                printf("ERROR (descA): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }


        char* bufA = (char*)malloc(20 * sizeof(char));
        char* znak = (char*)malloc(sizeof(char));

        int iter = 0;
        i = 0;
        int j = 0;
        while(read(descA, znak, 1) > 0){
                if(znak[0] == ','){
			printf("przecinek\n");
                        matrixA[i][j] = atof(bufA);
                        memset(bufA,0, strlen(bufA));
			j++;
			iter = 0;	
                }else if(znak[0] == '\n'){
			printf("enter\n");
                        matrixA[i][j] = atof(bufA);
                        memset(bufA,0, strlen(bufA));
                        i++;
                        j = 0;
			iter = 0;
                }else{
			printf("znak: %c\n", znak[0]);
                        bufA[iter] = znak[0];
			iter++;
                }
        }
	if(i == 3){
                matrixA[i][j] = atof(bufA);
	}
	free(bufA);	
	free(znak);

	printf("\nMatrixA:\n");
        for(i = 0; i<4; i++){
        	for(j = 0; j<4; j++){
                	printf("%f ",  matrixA[i][j]);
                }
        	printf("\n");
        }

	close(descA);
	return 0;

}
