all: serv client gen

serv: server.c matrixLoader.c division.c
	gcc -fopenmp server.c matrixLoader.c division.c -lm -o serv -Wall

client: client.c
	gcc -fopenmp client.c -o client -Wall
gen: matrixgen.c
	gcc matrixgen.c -o gen -Wall
