all: serv client gen

client: client.c matrixLoader.c division.c
	gcc -fopenmp client.c matrixLoader.c division.c -lm -o client -Wall

serv: server.c
	gcc -fopenmp server.c -o serv -Wall
gen: matrixgen.c
	gcc matrixgen.c -o gen -Wall
