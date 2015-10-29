all: serv client

serv: server.c matrixLoader.c
	gcc server.c matrixLoader.c -o serv

client: client.c
	gcc client.c -o client
