#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <pthread.h>
#include "msg.h"
extern int errno;
#define LISTENQ 5
#define MAX_BUF 10		/* Maximum bytes fetched by a single read() */
#define MAX_EVENTS 5
#define READING_REQUEST '0'
#define HEADER_PARSING '1'
#define READING_DISKFILE '2'
#define WRITING_HEADER '3'
#define WRITING_BODY '4'
#define DONE '5'		/* Maximum number of events to be returned from
				   a single epoll_wait() call */




void
errExit (char *s)
{
  perror (s);
  exit (1);
}








int epfd, ready, fd, s, j, num0penFds, msgqid, connfd;
key_t msgqkey;

struct epoll_event ev;
 struct epoll_event evlist[MAX_EVENTS];
  char buf[MAX_BUF];
  int listenfd, clilen;
  struct sockaddr_in cliaddr, servaddr;
  char cli_info[20][1000];
pthread_t tid1;



void serv_clnt()
{
	char http_header[] = "HTTP/1.1 200 OK\nContent-Type: plain/text\nContent-Length: 100\n\n";
	msg m;
	m.type = 1;
	int i;
	FILE* f;
	for(;;)
	{
	
		msgrcv(msgqid,&m,MSGSZ,0,NULL);
		
			switch(m.state)
		{
			
			case HEADER_PARSING:

			for(i=0;cli_info[m.hash_index][i] != '/';i++);
			i++;
			int j=0;
			for(;cli_info[m.hash_index][i]!=' ';i++)
			{
				cli_info[m.hash_index][j] = cli_info[m.hash_index][i];
				j++;
			}
			cli_info[m.hash_index][j] = '\0';
			m.state = WRITING_HEADER;
			msgsnd(msgqid,&m,MSGSZ,NULL);

			break;
			case READING_DISKFILE:
			f = fopen(cli_info[m.hash_index],"r");
			int n = read(fileno(f),&cli_info[m.hash_index],100);
			cli_info[m.hash_index][n] = '\0';
			write(m.connfd,cli_info[m.hash_index],100);
			break;
			case WRITING_HEADER:
			write(m.connfd,http_header,strlen(http_header));
			m.state = READING_DISKFILE;
			msgsnd(msgqid,&m,MSGSZ,NULL);

			break;




		
		}

		
	}

}







int
main (int argc, char *argv[])
{
	msg m;  
  m.type = 1;
  msgqkey = ftok("/bin",56);
  msgqid = msgget(msgqkey,IPC_CREAT|0666);
  for(j=0;j<20;j++)
  {
  	cli_info[j][0] = '\0';
  }

  pthread_create(&tid1, NULL, &serv_clnt, NULL);
  listenfd = socket (AF_INET, SOCK_STREAM, 0);
  bzero (&servaddr, sizeof (servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
  servaddr.sin_port = htons (atoi (argv[1]));

  if (bind (listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr)) < 0)
    perror ("bind");

  listen (listenfd, LISTENQ);


  if (argc < 2 || strcmp (argv[1], "--help") == 0)
    printf ("Usage: %s <port>\n", argv[0]);

  epfd = epoll_create (20);
  if (epfd == -1)
    errExit ("epoll_create");


  ev.events = EPOLLIN;		/* Only interested in input events */
  ev.data.fd = listenfd;
  if (epoll_ctl (epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
    errExit ("epoll_ctl");
  for (;;)
    {
  
      ready = epoll_wait (epfd, evlist, MAX_EVENTS, -1);
      if (ready == -1)
	{
	  if (errno == EINTR)
	    continue;		/* Restart if interrupted by signal */
	  else
	    errExit ("epoll_wait");
	}
      //printf("nready=%d\n", ready);

      for (j = 0; j < ready; j++)
	{
	  if (evlist[j].events & EPOLLIN)
	    {
	      if (evlist[j].data.fd == listenfd)
		{
		  clilen = sizeof (cliaddr);
		  char ip[128];
		  memset (ip, '\0', 128);
		  connfd =
		    accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

		  if (cliaddr.sin_family == AF_INET)
		    {
		      if (inet_ntop (AF_INET, &(cliaddr.sin_addr), ip, 128) ==
			  NULL)
			perror ("Error in inet_ntop\n");
		    }

		  if (cliaddr.sin_family == AF_INET6)
		    {
		      inet_ntop (AF_INET6, &(cliaddr.sin_addr), ip, 128);
		    }

		  printf ("new client: %s, port %d\n", ip,
			  ntohs (cliaddr.sin_port));


		  ev.events = EPOLLIN;	/* Only interested in input events */
		  ev.data.fd = connfd;
		  if (epoll_ctl (epfd, EPOLL_CTL_ADD, connfd, &ev) == -1)
		    errExit ("epoll_ctl");
		}
		else
		{

	      connfd = evlist[j].data.fd;
		}
			int i;
		  	m.connfd = connfd;
			for(i=0;i<20;i++)
			{
				if(cli_info[i][0] == '\0')
				{
					int n = read(m.connfd,&cli_info[i][0],1000);
					cli_info[i][n] = '\0';
					m.hash_index = i;
					m.state = HEADER_PARSING;
					msgsnd(msgqid,&m,MSGSZ,NULL);
					printf("message sent...\n");
					break;
				}
			}
		  	

		
	    }
	}
    }
}

