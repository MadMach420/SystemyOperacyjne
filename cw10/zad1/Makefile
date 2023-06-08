all: server client

server: server.c
	gcc -Wall -pthread -o server server.c

client: client.c
	gcc -Wall -pthread -o client client.c

clean:
	rm client server