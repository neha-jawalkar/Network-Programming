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

#define NUMSEMS 3
#define SIZEOFSHMSEG 1024

int main()
{
	char* shmaddr;
	struct sembuf ops[2];
	int semid = semget(IPC_PRIVATE, NUMSEMS, 0666);
	semctl(semid, 0, SETVAL, 0);
	semctl(semid, 1, SETVAL, 0);
	semctl(semid, 2, SETVAL, 0);
	int shmid = shmget(IPC_PRIVATE, SIZEOFSHMSEG, 0666);
	shmaddr = (char*)shmat(shmid,NULL,0);
	pid_t a,b,c;
	a = fork();
	if(a < 0)
	{
		perror("Fork.\n");
		return 0;
	}
	else if(a > 0)
	{
		b = fork();
		if(b < 0)
		{
			perror("Fork.\n");
			return 0;
		}
		else if(b > 0)
		{
			c = fork();
			if(c < 0)
			{
				perror("Fork.\n");
				return 0;
			}
			else if(c > 0)
			{
				pid_t ret = wait(NULL);
				if(ret == a)
				{
					kill(b,SIGTERM);
					kill(c,SIGTERM);
					return 0;
				}
				else if(ret == b)
				{
					kill(a,SIGTERM);
					kill(c,SIGTERM);
					return 0;
				}
				else if(ret == c)
				{
					kill(a,SIGTERM);
					kill(b,SIGTERM);
					return 0;
				}
			}
			else if(c == 0)
			{
				while(1)
				{		
					ops[0].sem_num = 1;
					ops[0].sem_op = -1;
					ops[0].sem_flg = 0;
					ops[1].sem_num = 2;
					ops[1].sem_op = -1;
					ops[1].sem_flg = 0;
					if(semop(semid,ops,2) == -1)
					{
						perror("Semop.\n");
						return 0;
					}
					printf("PID: %d ACTION: print\n%s\n",getpid(),(char*)shmaddr);
					ops[0].sem_num = 1;
					ops[0].sem_op = -1;
					ops[0].sem_flg = 0;
					ops[1].sem_num = 2;
					ops[1].sem_op = -1;
					ops[1].sem_flg = 0;
					if(semop(semid,ops,2) == -1)
					{
						perror("Semop.\n");
						return 0;
					}
				}
			}
		}
		else if(b == 0)
		{
			while(1)
			{
				ops[0].sem_num = 0;
				ops[0].sem_op = -1;
				ops[0].sem_flg = 0;
				ops[1].sem_num = 1;
				ops[1].sem_op = 0;
				ops[1].sem_flg = 0;
				if(semop(semid,ops,2) == -1)
				{
					perror("Semop.\n");
					return 0;
				}
				int i, len = strlen((char*)shmaddr);
				for(i=0; i < len; i++)
				{
					shmaddr[i] = toupper(shmaddr[i]);
				}
				printf("PID: %d ACTION: capitalize\n%s\n",getpid(),(char*)shmaddr);
				ops[0].sem_num = 0;
				ops[0].sem_op = -1;
				ops[0].sem_flg = 0;
				ops[1].sem_num = 1;
				ops[1].sem_op = 2;
				ops[1].sem_flg = 0;
				if(semop(semid,ops,2) == -1)
				{
					perror("Semop.\n");
					return 0;
				}
			}
		}
	}
	else if(a == 0)
	{
		while(1)
		{
			ops[0].sem_num = 0;
			ops[0].sem_op = 0;
			ops[0].sem_flg = 0;
			ops[1].sem_num = 2;
			ops[1].sem_op = 0;
			ops[1].sem_flg = 0;
			if(semop(semid,ops,2) == -1)
			{
				perror("Semop.\n");
				return 0;
			}
			printf("PID: %d ACTION: read\n",getpid());
			fgets(shmaddr,SIZEOFSHMSEG,stdin);
			shmaddr[strlen((char*)shmaddr)-1] = '\0';
			if(feof(stdin))
				return 0;
			ops[0].sem_num = 0;
			ops[0].sem_op = 2;
			ops[0].sem_flg = 0;
			ops[1].sem_num = 2;
			ops[1].sem_op = 2;
			ops[1].sem_flg = 0;
			if(semop(semid,ops,2) == -1)
			{
				perror("Semop.\n");
				return 0;
			}
		}
	}
}