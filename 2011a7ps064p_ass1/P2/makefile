all: shmserver shmclient

shmserver: message.o shmserver.c key.h
	gcc -Wall message.o shmserver.c -o shmserver

shmclient: message.o shmclient.c key.h
	gcc -Wall message.o shmclient.c -o shmclient

message.o: message.c message.h key.h
	gcc -Wall message.h -c message.c

clean:
	rm *.o

