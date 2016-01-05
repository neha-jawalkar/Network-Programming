#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include  <math.h>
#include <stdbool.h>
#include <sys/msg.h>
#include "rshminfo.h"
#include "msg.h"
socklen_t clnt_len;
struct sockaddr_in serv_addr;
struct sockaddr_in clnt_addr;
int listenSock, clntSock,rshmkey,found_index,free_index,shmid,rshmid,i;
size_t rshmsize;
rshminfo rshminfo_array[MAX_ALLOWED_MEMORY_SEGMENTS];
msg m;
void fatal(char* message)
{
	perror(message);
	exit(1);
}



void setup(int argc,char* argv[])
{
	for(i=0;i<MAX_ALLOWED_MEMORY_SEGMENTS;i++)
	{
		rshminfo_array[i].rshmid = -1;
	}
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	if(serv_addr.sin_addr.s_addr == -1)
		fatal("error sock");
	serv_addr.sin_port = htons(atoi(argv[1]));
	if ((listenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		fatal("socket() failed");
	if (bind(listenSock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		fatal("bind() failed");
	if (listen(listenSock, MAXPENDING) < 0)
		fatal("listen() failed");
	


}

void find(int attr)
{
	free_index = -1;
	int found_free_index = 0;
	for(i=0;i<MAX_ALLOWED_MEMORY_SEGMENTS;i++)
	{
		if((attr == 0 && rshminfo_array[i].key == rshmkey) || (attr == 1 && rshminfo_array[i].rshmid == rshmid))
		{
			/*rcvBuffer[0] = '\0';
			strcat(rcvBuffer,itoa(rshminfo_array[i].rshmid));
			if (send(clntSocket, rcvBuffer, recvMsgSize, 0) != strlen(rcvBuffer))
				fatal("send() failed");*/
			free_index = -1;
			found_index = i;
			return;
		}
		else if(found_free_index == 0 && rshminfo_array[i].rshmid == -1)
		{
			free_index = i;
			found_free_index = 1;
		}
	}
	found_index = -1;
	return;
}
void create()
{
	if(free_index == -1)
	{
			fatal("No more space in memory.\n");
	}
	shmid = shmget(rshmkey, rshmsize, IPC_CREAT|0666);
	if(shmid == -1)
		fatal("shmid()");
	rshminfo_array[free_index].rshmid = m.rshmid;
	rshminfo_array[free_index].key = m.rshmkey;
	rshminfo_array[free_index].shmid = shmid;
	rshminfo_array[free_index].addr = shmat(shmid,NULL,0);
	rshminfo_array[free_index].size = m.rshmsize;
	rshminfo_array[free_index].ref_count = 1;
}

void send_state_table_to_remote_endpoint()
{
	for(i=0;i<MAX_ALLOWED_MEMORY_SEGMENTS;i++)
	{
		send(clntSock,&rshminfo_array[i],sizeof(rshminfo),0);
		send(clntSock,rshminfo_array[i].addr,rshminfo_array[i].size,0);


	}
}



void serv_remote_endpoint()
{
	char cmd = m.cmd;
	rshmkey = m.rshmkey;
	rshmid = m.rshmid;
	switch(cmd)
	{
		case CONSTANT_RSHMGET_CHAR:
			create();
			break;
		case CONSTANT_RSHMAT_CHAR:
			find(1);
			rshminfo_array[found_index].ref_count++;
			break;
		case CONSTANT_RSHMDT_CHAR:
			find(1);
			rshminfo_array[found_index].ref_count--;
			if(rshminfo_array[found_index].ref_count == 0 &&  rshminfo_array[found_index].rm == 1)
				rshminfo_array[found_index].rshmid = -1;
			break;
		case CONSTANT_RSHMCTL_CHAR:
			find(1);
			rshminfo_array[found_index].rm = 1;
			shmctl(rshminfo_array[found_index].shmid,IPC_RMID,NULL);
			break;
		case CONSTANT_RSHMCHANGED_CHAR:
			find(1);
			recv(clntSock,rshminfo_array[found_index].addr,rshminfo_array[found_index].size,0);
			break;
		case CONSTANT_MESSAGE_FROM_REMOTE_ENDPOINT_CHAR:
			send_state_table_to_remote_endpoint();

	}
}

int main(int argc, char *argv[])
{
	setup(argc,argv);
	for(;;)
	{
		
		if ((clntSock = accept(listenSock, (struct sockaddr *) &clnt_addr,&clnt_len)) != -1)
		{

			if (recv(clntSock, &m, MSGSZ, 0) < 0)
				fatal("recv() failed");
			serv_remote_endpoint();
			close(clntSock);
		}
/*		else
		{
			printf("%d\n",clnt)
			fatal("accept() failed");
		}*/
	}
}

