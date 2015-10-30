#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
int main(){
  
  //macierz A
  int desc = open("Amatrix", O_WRONLY | O_TRUNC | O_CREAT, 777);
  
  int szer[1];
  int wys[1];
  szer[0] = 2000;
  wys[0] = 2000;
  int i, j;

  write(desc, wys, 4);
  write(desc, szer, 4);

  float* buff = (float*) malloc (szer[0]*sizeof(float));  
  for(i = 0; i < wys[0]; i++){
      for(j = 0; j < szer[0]; j++){
		buff[j] = (float)rand()/(float)(RAND_MAX/1.2);
	}
	write(desc, buff, 4*szer[0]);
  }
  close(desc);
  free(buff);
  //macierz B
  int desc2 = open("Bmatrix", O_WRONLY | O_TRUNC | O_CREAT, 777);

  szer[0] = 2000;
  wys[0] = 2000;

  write(desc2, wys, 4);
  write(desc2, szer, 4);

  float* buff2 = (float*) malloc (szer[0]*sizeof(float));
  for(i = 0; i < wys[0]; i++){
      for(j = 0; j < szer[0]; j++){
        	buff2[j] = (float)rand()/(float)(RAND_MAX/1.2);
        }
	write(desc2, buff2, 4*szer[0]);
      
  }
  free(buff2);
  close(desc2);
  return 0;
}
