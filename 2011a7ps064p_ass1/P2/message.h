#include <stdio.h>
#include <stdlib.h>

#define NUMSEMS 4
#define MAX_ALLOWED_MESSAGES 2
#define MSGZ sizeof(message)
#define SIZEOFSHMSEG MAX_ALLOWED_MESSAGES*MSGZ
#define MAX_ALLOWED_CLIENTS	100

typedef struct message{
int type;
int pid; //client's pid
int slno; //incremented for every message
int a; //any number
int b; //any number
int total;//total of a and b, processed by server
}message;
void fatal(char* message);