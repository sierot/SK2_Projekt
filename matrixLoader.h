#ifndef matrixLoader_h
#define matrixLoader_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

void loadFile(char* , float** , int*);
void loadSize(char* , int*);

#endif

