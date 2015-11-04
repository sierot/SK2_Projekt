#include "matrixLoader.h"

void loadSize(char* path, int* lincol){
	//otwieramy deskryptor pliku
	int desc = open(path, O_RDONLY);
        if(desc < 0){
                printf("ERROR (desc): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }

	read(desc, lincol, 8);

	//sprzatamy po sobie
	close(desc);
}

void loadFile(char* path, float** matrix, int* size){
	
	//otwieranie deskryptora pliku	
	int desc = open(path, O_RDONLY);
        if(desc < 0){
                printf("ERROR (desc): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
        
	lseek(desc, 8, SEEK_SET);
	
	int i;
	for(i = 0; i<size[0]; i++){
		read(desc, matrix[i], 4*size[1]);
	}
	//sprzatamy po sobie :)
	close(desc);
}
