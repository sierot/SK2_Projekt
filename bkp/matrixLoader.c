#include "matrixLoader.h"

void loadSize(char* path, int* lincol){
	int desc = open(path, O_RDONLY);
        if(desc < 0){
                printf("ERROR (desc): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }

        char* buf = (char*)malloc(20 * sizeof(char));
        char* c = (char*)malloc(sizeof(char));

        int index = 0;
        int i = 0;
        int j = 0;
        while(read(desc, c, 1) > 0){
                if(c[0] == ','){
                        lincol[0] = atoi(buf);
                        memset(buf,0, strlen(buf));
                        index = 0;
                }else if(c[0] == '\n'){
                        lincol[1] = atoi(buf);
                        memset(buf,0, strlen(buf));
                        break;
                }else{
                        buf[index] = c[0];
                        index++;
                }
        }
	close(desc);
	free(buf);
	free(c);
}

void loadFile(char* path, float** matrix, int* size){
	
	int desc = open(path, O_RDONLY);
        if(desc < 0){
                printf("ERROR (desc): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }


	char* buf = (char*)malloc(20 * sizeof(char));
        char* c = (char*)malloc(sizeof(char));
	
	while(read(desc, c, 1) > 0){
		if(c[0] == '\n') break;
	}

        int index = 0;
        int i = 0;
        int j = 0;
        while(read(desc, c, 1) > 0){
                if(c[0] == ','){
                        matrix[i][j] = atof(buf);
                        memset(buf,0, strlen(buf));
                        j++;
                        index = 0;
                }else if(c[0] == '\n'){
                        matrix[i][j] = atof(buf);
                        memset(buf,0, strlen(buf));
                        i++;
                        j = 0;
                        index = 0;
                }else{
                        buf[index] = c[0];
                        index++;
                }
        }
        if(i == size[0]-1){
                matrix[i][j] = atof(buf);
        }
	close(desc);
        free(buf);
        free(c);
}
