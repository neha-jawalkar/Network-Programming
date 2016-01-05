#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "message.h"
#include "key.h"


//remember to check for o_create in semaphores
int main(int argc, char* argv[])
{
	message* shmaddr;
	struct sembuf ops[4];
	int semid;
	int nummessages = MAX_ALLOWED_MESSAGES;
	key_t semkey, shmkey;
	semkey = ftok(SEM_PATH,SEM_ID);
	semid = semget(semkey, NUMSEMS, 0666);
	if(semid == -1)
	{
		fatal("semget()\n");
	}
	shmkey = ftok(SHM_PATH,SHM_ID);
	int shmid = shmget(shmkey, SIZEOFSHMSEG, 0666);
	if(shmid == -1)
	{
		fatal("shmget failed.\n");
	}
	shmaddr = (message*)shmat(shmid,NULL,0);
	ops[0].sem_num = 3;
	ops[0].sem_op = 1;
	ops[0].sem_flg = 0;
	if(semop(semid,ops,1) == -1)
	{
		fatal("Semop1.\n");
	}
	ops[0].sem_num = 0;
	ops[0].sem_op = 0;
	ops[0].sem_flg = 0;
	ops[1].sem_num = 0;
	ops[1].sem_op = 1;
	ops[1].sem_flg = 0;
	ops[2].sem_num = 1;
	ops[2].sem_op = -2;
	ops[2].sem_flg = 0;
	if(semop(semid,ops,3) == -1)
	{
		fatal("Semop1.\n");
	}
//	printf("boo\n");
	int i;
	int j,k;
	int start_index=0;
	for(i=1;i < argc;)
	{
		for(j = 0; j < nummessages && i < argc; j++)
		{
			if(shmaddr[j].type == 0)
			{
				shmaddr[j].type = 1;
				shmaddr[j].pid = getpid();
				shmaddr[j].slno = i/2;
				shmaddr[j].a = atoi(argv[i]);
				shmaddr[j].b = atoi(argv[i+1]);
				printf("LABEL : CLIENT PID : %d SLNO : %d A : %d B : %d SHMID : %d SEM1 : %d SEM2 : %d SEM3 : %d SEM4 : %d\n",getpid(), shmaddr[j].slno,shmaddr[j].a,shmaddr[j].b,shmid,semctl(semid,0,GETVAL,0),semctl(semid,1,GETVAL,0),semctl(semid,2,GETVAL,0),semctl(semid,3,GETVAL,0));
			}
		i=i+2;
		}
		ops[0].sem_num = 1;
		ops[0].sem_op = 1;
		ops[0].sem_flg = 0;
		ops[1].sem_num = 0;
		ops[1].sem_op = -1;
		ops[1].sem_flg = 0;
		if(semop(semid,ops,2) == -1)
		{
			fatal("Semop2.\n");
		}
		for(j = start_index; j < i/2; j++)
		{
			ops[0].sem_num = 0;
			ops[0].sem_op = 0;
			ops[0].sem_flg = 0;
			ops[1].sem_num = 0;
			ops[1].sem_op = 1;
			ops[1].sem_flg = 0;
			ops[2].sem_num = 2;
			ops[2].sem_op = -getpid();
			ops[2].sem_flg = 0;
			ops[3].sem_num = 2;
			ops[3].sem_op = 0;
			ops[3].sem_flg = 0;
			if(semop(semid,ops,4) == -1)
			{
				fatal("Semop3.\n");
			}
			for(k = 0;k < nummessages ; k++)
			{
				if(shmaddr[k].type == getpid())
				{
					shmaddr[k].type = 0;
					break;
				}
			}
			ops[0].sem_num = 1;
			ops[0].sem_op = semctl(semid,1,GETVAL,0) == 1 ? 1 : 2;
			ops[0].sem_flg = 0;
			ops[1].sem_num = 0;
			ops[1].sem_op = -1;
			ops[1].sem_flg = 0;
			if(semop(semid,ops,2) == -1)
			{
				fatal("Semop4.\n");
			}
		}
		start_index=i;
	}
	ops[0].sem_num = 3;
	ops[0].sem_op = -1;
	ops[0].sem_flg = 0;
	if(semop(semid,ops,1) == -1)
	{
		fatal("Semop1.\n");
	}
	return 0;
}