#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/msg.h>
#include "rshminfo.h"
#include "msg.h"
int msgqkey[2];
int msgqid[2];
int MSGTYP;
msg m[2];
void fatal(char* message)
{
	perror(message);
	exit(1);
}

void setup(char* argv[])
{
	msgqkey[0] = ftok(argv[1],atoi(argv[2]));
	msgqid[0] = msgget(msgqkey[0],IPC_CREAT|0666);
	msgqkey[1] = ftok(argv[3],atoi(argv[4]));
	msgqid[1] = msgget(msgqkey[1],IPC_CREAT|0666);
	MSGTYP = getpid();
	m[0].type = MSGTYP;
}



void prepare(char c)
{
	m[0].cmd = c;
}

int rshmget(key_t key, size_t size)
{
	
	prepare(CONSTANT_RSHMGET_CHAR);
	m[0].rshmkey = key;
	m[0].rshmsize = size;
	msgsnd(msgqid[0],&m[0],MSGSZ,0);
	msgrcv(msgqid[1],&m[1],MSGSZ,MSGTYP,0);
	return m[1].rshmid;
	
}

void* rshmat(int rshmid, void* addr)
{

	prepare(CONSTANT_RSHMAT_CHAR);
	m[0].rshmid = rshmid;
	msgsnd(msgqid[0],&m[0],MSGSZ,0);
	msgrcv(msgqid[1],&m[1],MSGSZ,MSGTYP,0);
	return shmat(m[1].rshmid,addr,0);	
}

int rshmdt(int rshmid, void* addr)
{
	prepare(CONSTANT_RSHMDT_CHAR);
	m[0].rshmid = rshmid;
	msgsnd(msgqid[0],&m[0],MSGSZ,0);
	return shmdt(addr);
	

}

int rshmctl(int rshmid, int cmd)
{
	prepare(CONSTANT_RSHMCTL_CHAR);
	m[0].rshmid = rshmid;	
	msgsnd(msgqid[0],&m[0],MSGSZ,0);
	msgrcv(msgqid[1],&m[1],MSGSZ,MSGTYP,0);
	return m[1].retval;
}

void rshmChanged(int rshmid)
{
	prepare(CONSTANT_RSHMCHANGED_CHAR);																																																																																																									
	m[0].rshmid = rshmid;
	msgsnd(msgqid[0],&m[0],MSGSZ,0);
	return;
}

