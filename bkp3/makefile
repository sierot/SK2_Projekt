all: serv client gen

serv: server.c matrixLoader.c division.c
	gcc server.c matrixLoader.c division.c -lm -o serv

client: client.c
	gcc -fopenmp client.c -o client
gen: matrixgen.c
	gcc matrixgen.c -o gen
