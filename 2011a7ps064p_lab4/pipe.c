#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
pid_t c1,c2;
//remove libraries you don't need
//don't close one end of a pipe before it has been duplicated
int main()
{
	int fd[4][2];
	char s[500];
	char o[500];
	pipe(fd[0]);
	pipe(fd[1]);
	c1 = fork();
	if(c1 < 0)
	{
		perror("Fork.\n");
		return 0;
	}
	else if(c1 > 0)
	{
		close(fd[0][0]);
		close(fd[1][1]);
		pipe(fd[2]);
		pipe(fd[3]);
		c2 = fork();
		if(c2 < 0)
		{
			perror("Fork.\n");
			kill(c1,SIGTERM);
			return 0;
		}
		else if(c2 > 0)
		{
			close(fd[2][0]);
			close(fd[3][1]);
			while(1)
			{
				fgets(s,sizeof(s),stdin);
				s[strlen(s)-1] = '\0';
				if(feof(stdin))
				{
					kill(c1,SIGTERM);
					kill(c2,SIGTERM);
					return 0;
				}
				write(fd[0][1],s,strlen(s)+1);
				read(fd[1][0],o,sizeof(o));
				int l = strlen(o);
				write(fd[2][1],o,l+1);
				read(fd[3][0],o,sizeof(o));
				printf("%s\n",o);
			}
		}
		else if(c2 == 0)
		{
			close(fd[2][1]);
			close(fd[3][0]);
			while(1)
			{
				read(fd[2][0],s,sizeof(s));
				int l = strlen(s);
				s[l] = 'C';
				s[l+1] = '2';
				s[l+2] = '\0';
				write(fd[3][1],s,l+3);
			}
			
		}

	}
	else if(c1 == 0)
	{
		close(fd[0][1]);
		close(fd[1][0]);
		while(1)
			{
				read(fd[0][0],s,sizeof(s));
				int l = strlen(s);
				s[l] = 'C';
				s[l+1] = '1';
				s[l+2] = '\0';
				write(fd[1][1],s,l+3);
			}
	}

}