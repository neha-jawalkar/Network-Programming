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




int pid_count[MAX_ALLOWED_CLIENTS][2];
int i;
void shm_server_exit_handler(int signo)  
{  
  /* Cleanup Code Here */  
	for(i=0;i<MAX_ALLOWED_CLIENTS && pid_count[i][0]!=0;i++)
	{
		printf("PID : %d COUNT : %d\n",pid_count[i][0],pid_count[i][1]);
	}
	exit(0);
}  


int main(int argc, char* argv[])
{
	signal(SIGINT,shm_server_exit_handler);
	message* shmaddr;
	struct sembuf ops[3];
	int semid;
	int nummessages = SIZEOFSHMSEG/sizeof(message);
	key_t semkey, shmkey;
	semkey = ftok(SEM_PATH,SEM_ID);
	semid = semget(semkey, NUMSEMS, IPC_CREAT|0666);
	if(semid == -1)
	{
		fatal("semget()");
	}
	semctl(semid, 0, SETVAL, 0);
	semctl(semid, 1, SETVAL, 2);
	semctl(semid, 2, SETVAL, 0);
	semctl(semid, 3, SETVAL, 0);
	shmkey = ftok(SHM_PATH,SHM_ID);
	int shmid = shmget(shmkey, SIZEOFSHMSEG, IPC_CREAT|0666);
	if(shmid == -1)
	{
		fatal("shmget()");
	}
	shmaddr = (message*)shmat(shmid,NULL,0);
	memset(shmaddr,0,SIZEOFSHMSEG);
	
	for(;;)
	{

			ops[0].sem_num = 0;
			ops[0].sem_op = 0;
			ops[0].sem_flg = 0;
			ops[1].sem_num = 0;
			ops[1].sem_op = 1;
			ops[1].sem_flg = 0;
			ops[2].sem_num = 1;
			ops[2].sem_op = -1;
			ops[2].sem_flg = 0;
			if(semop(semid,ops,3) == -1)
			{
				fatal("Semop()");
			}

		

		for(i=0;i<nummessages;i++)
		{
			if(shmaddr[i].type == 1)
			{
				
				shmaddr[i].total = shmaddr[i].a + shmaddr[i].b;
				shmaddr[i].type = shmaddr[i].pid;
				//server’s pid (with label “server”), slno, a, b, sum of a and b, shmid, and semaphore value
				printf("LABEL : SERVER PID : %d SLNO : %d A : %d B : %d SUM : %d SHMID : %d SEM1 : %d SEM2 : %d SEM3 : %d SEM4 : %d\n",getpid(), shmaddr[i].slno, shmaddr[i].a,shmaddr[i].b,shmaddr[i].total,shmid,semctl(semid,0,GETVAL,0),semctl(semid,1,GETVAL,0),semctl(semid,2,GETVAL,0),semctl(semid,3,GETVAL,0));
				int j, found_pid=0;
				for(j=0;j<MAX_ALLOWED_CLIENTS && pid_count[j][0] != 0;j++)
				{
					if(pid_count[j][0] == shmaddr[i].pid)
					{
						pid_count[j][1]++;
						found_pid=1;
						break;
					}
				}
				if(found_pid==0 && j!=MAX_ALLOWED_CLIENTS)
				{
					pid_count[j][0] = shmaddr[i].pid;
					pid_count[j][1] = 1;
				}
				ops[0].sem_num = 2;
				ops[0].sem_op = shmaddr[i].pid;
				ops[0].sem_flg = 0;
				ops[1].sem_num = 0;
				ops[1].sem_op = -1;
				ops[1].sem_flg = 0;
				if(semop(semid,ops,2) == -1)
				{
					fatal("Semop()");
				}
				break;
			}
		}
		if(i==nummessages)
		{
			ops[0].sem_num = 1;
			ops[0].sem_op = 1;
			ops[0].sem_flg = 0;
			ops[1].sem_num = 0;
			ops[1].sem_op = -1;
			ops[1].sem_flg = 0;
			if(semop(semid,ops,2) == -1)
			{
				fatal("Semop()");
			}
			while(semctl(semid,3,GETVAL,0) == 0);
		}
		

	}
	return 0;
}