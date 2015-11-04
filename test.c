#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
  
  //macierz A
  int desc = open("Amatrix", O_WRONLY | O_TRUNC | O_CREAT, 0666);
  
  int szer[1];
  int wys[1];
  wys[0] = 4;
  szer[0] = 6;
  int i, j;

  write(desc, wys, 4);
  write(desc, szer, 4);

  float* buff = (float*) malloc (szer[0]*sizeof(float));  
  for(i = 0; i < wys[0]; i++){
      for(j = 0; j < szer[0]; j++){
		buff[j] = i*4 + j;
	
		//buff[j] = 0.0;//(float)rand()/(float)(RAND_MAX/0.2);
	}
	write(desc, buff, 4*szer[0]);
  }
  close(desc);
  free(buff);
  //macierz B
  int desc2 = open("Bmatrix", O_WRONLY | O_TRUNC | O_CREAT, 0666);
  wys[0] = 4;
  szer[0] = 6;

  write(desc2, wys, 4);
  write(desc2, szer, 4);

  float* buff2 = (float*) malloc (szer[0]*sizeof(float));
  for(i = 0; i < wys[0]; i++){
      for(j = 0; j < szer[0]; j++){
		  buff2[j] = i*4 + j;
	  //buff2[j] = 0.0;//(float)rand()/(float)(RAND_MAX/0.2);
        }
	write(desc2, buff2, 4*szer[0]);
      
  }
  free(buff2);
  close(desc2);
  return 0;
}
