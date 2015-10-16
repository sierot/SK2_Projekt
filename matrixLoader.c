#include "matrixLoader.h"

void loadSize(char* path, int* lincol){
	//otwieramy deskryptor pliku
	int desc = open(path, O_RDONLY);
        if(desc < 0){
                printf("ERROR (desc): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }

	//alokacja pamieci potrzebej do odczytania rozmiaru
        char* buf = (char*)malloc(20 * sizeof(char));
        char* c = (char*)malloc(sizeof(char));

        int index = 0;
        int i = 0;
        int j = 0;
	//oczytujemy rozmiar macierzy
        while(read(desc, c, 1) > 0){
                if(c[0] == ','){
                        lincol[0] = atoi(buf); //do przecinka sa wiersze
                        memset(buf,0, strlen(buf)); //zerujemy bufor
                        index = 0; //zerujemy indeks w buforze
                }else if(c[0] == '\n'){
                        lincol[1] = atoi(buf); //do \n sa kolumny
                        memset(buf,0, strlen(buf)); //zerujemy bufor
                        break;
                }else{
                        buf[index] = c[0]; //zerujemy bufor
                        index++; //zwiekszamy indeks w buforze
                }
        }
	//sprzatamy po sobie
	close(desc);
	free(buf);
	free(c);
}

void loadFile(char* path, float** matrix, int* size){
	
	//otwieranie deskryptora pliku	
	int desc = open(path, O_RDONLY);
        if(desc < 0){
                printf("ERROR (desc): %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }

	//alokacja pamieci do prawidlowego odczytu z pliku
	char* buf = (char*)malloc(20 * sizeof(char));
        char* c = (char*)malloc(sizeof(char));
	
	//pomijamy pierwszy wiersz, bo tam jest rozmiar macierzy
	while(read(desc, c, 1) > 0){
		if(c[0] == '\n') break;
	}

        int index = 0;
        int i = 0;
        int j = 0;
	//odczytujemy znak po znaku i odpowiednio rozdzielamy dane
        while(read(desc, c, 1) > 0){
                if(c[0] == ','){
                        matrix[i][j] = atof(buf); //co uzbieralo sie w buforze to nasz float
                        memset(buf,0, strlen(buf)); //zerujemy bufor
                        j++; //przygotowujemy sie do nastepnego floata w wierszu
                        index = 0; //zerujemy indeks w buforze
                }else if(c[0] == '\n'){
                        matrix[i][j] = atof(buf); //dane z bufora -> macierzy
                        memset(buf,0, strlen(buf)); //zerujemy bufor
                        i++; //przygotowujemy sie do nastepnego wierszu
                        j = 0; //w nowym wierszu od poczatku
                        index = 0; //zerujemy indeks w buforze
                }else{
                        buf[index] = c[0]; //uzupelniamy bufor
                        index++; //zwiekszamy indeks w buforze
                }
        }
	//jezeli plik nie byl zakonczony \n
        if(i == size[0]-1){
                matrix[i][j] = atof(buf);
        }
	//sprzatamy po sobie :)
	close(desc);
        free(buf);
        free(c);
}
