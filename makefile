all: serv client

serv: server.c matrixLoader.c division.c
	gcc server.c matrixLoader.c division.c -lm -o serv

client: client.c
	gcc client.c -o client
