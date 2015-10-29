#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

int main(){

	float** matrixA = (float**)malloc(11 * sizeof(float*));
        int i;
        for(i = 0; i<11; i++){
        	matrixA[i] = (float*)malloc(4 * sizeof(float));
        }


	int descA = open("Bmatrix", O_RDONLY);
        if(descA < 0){
                printf("ERROR (descA): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }

	int tab[2];
	read(descA, tab, 8);
	printf("%d\n", tab[0]);
	printf("%d\n", tab[1]);
        
	for(i = 0; i < tab[0]; i++){
		read(descA, matrixA[i], 4*tab[1]);
	 }
	
	int j;
	for(i = 0; i < tab[0]; i++){
		for(j = 0; j < tab[1]; j++){
			printf("%f ", matrixA[i][j]);
		}
		printf("\n");
	}
	free(matrixA);
	close(descA);
	return 0;

}
