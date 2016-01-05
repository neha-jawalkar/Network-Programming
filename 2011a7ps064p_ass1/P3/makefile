all: rshmsever_main rshmserver_aux p1

rshmsever_main: msg.h rshminfo.h rshmserver_main.c
	gcc -Wall rshmserver_main.c -o sm

rshmserver_aux: msg.h rshminfo.h rshmserver_main.c
	gcc -Wall rshmserver_aux.c -o sa

p1: msg.h rshmAPI.o rshminfo.h p1.c
	gcc -Wall rshmAPI.o p1.c -o p1

rshmAPI.o: msg.h rshmAPI.c rshmAPI.h
	gcc -Wall rshmAPI.h -c rshmAPI.c

clean:
	rm *.o

