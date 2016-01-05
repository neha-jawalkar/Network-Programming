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
//remember to close the socket with teardown()
 /* list of remote end
points*/

struct sockaddr_in serv_addr;
int servSock, rshmkey,found_index,free_index,shmid,rshmid,flags,i;
int msgqid[2];
int msgqkey[2];
size_t rshmsize;
rshminfo rshminfo_array[MAX_ALLOWED_MEMORY_SEGMENTS];
msg m[2];
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
	msgqkey[0] = ftok(argv[1],atoi(argv[2]));
	msgqid[0] = msgget(msgqkey[0],IPC_CREAT|0666);
	msgqkey[1] = ftok(argv[3],atoi(argv[4]));
	msgqid[1] = msgget(msgqkey[1],IPC_CREAT|0666);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(inet_addr(argv[5]));
	if(serv_addr.sin_addr.s_addr == -1)
		fatal("error sock");
	serv_addr.sin_port = htons(atoi(argv[6]));

}

void find(int attr)
{
	rshmid = m[0].rshmid;
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
	rshmid = rand();
	rshminfo_array[free_index].rshmid = rshmid;
	rshminfo_array[free_index].key = rshmkey;
	rshminfo_array[free_index].shmid = shmid;
	rshminfo_array[free_index].addr = shmat(shmid,NULL,0);
	rshminfo_array[free_index].size = rshmsize;
	rshminfo_array[free_index].ref_count = 1;
}

void send_to_main_server(int send_mem_segment)
{
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		fatal("socket() failed");
	flags = fcntl(servSock, F_GETFL, 0);
	fcntl(servSock, F_SETFL, flags | O_NONBLOCK);
	if (connect(servSock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) >= 0)
	{
		if (send(servSock, &m[0], MSGSZ, 0) != MSGSZ)
			fatal("send() sent a different number of bytes than expected");
		if(send_mem_segment == 1)
		{
			if (send(servSock, rshminfo_array[found_index].addr, rshminfo_array[found_index].size , 0) != rshminfo_array[found_index].size)
				fatal("send() sent a different number of bytes than expected");
		}
	}		
	close(servSock);
}

void send_to_client()
{
	msgsnd(msgqid[1],&m[1],MSGSZ,0);
}

void serv_rshmget()
{
	
   /* get the first token */
	rshmkey = m[0].rshmkey;
	rshmsize = m[0].rshmsize;
	find(0);
	if(found_index == -1)
	{
		create();	
 		send_to_main_server(0);

	}
	else
	{
		rshmid = rshminfo_array[found_index].rshmid;
	}
	m[1].type = m[0].type;
	m[1].rshmid = rshmid;
	send_to_client();
}

void serv_rshmat()
{
	find(1);
	rshminfo_array[found_index].ref_count++;
	send_to_main_server(0);
	m[1].type = m[0].type;
	m[1].rshmid = rshminfo_array[found_index].shmid;
	send_to_client();

}

void serv_rshmdt()
{
	find(1);
	rshminfo_array[found_index].ref_count--;
	if(rshminfo_array[found_index].ref_count == 0 &&  rshminfo_array[found_index].rm == 1)
		rshminfo_array[found_index].rshmid = -1;
	send_to_main_server(0);

}

void serv_rshmctl()
{
	rshminfo_array[found_index].rm = 1;
	m[1].retval = shmctl(rshminfo_array[found_index].shmid,IPC_RMID,NULL);
	send_to_main_server(0);
	m[1].type = m[0].type;
	send_to_client();


}

void serv_rshmchanged()
{
	find(1);
	send_to_main_server(1);
}

void get_info_from_main_server()
{
	m[0].cmd = CONSTANT_MESSAGE_FROM_REMOTE_ENDPOINT_CHAR;
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		fatal("socket() failed");
	flags = fcntl(servSock, F_GETFL, 0);
	fcntl(servSock, F_SETFL, flags | O_NONBLOCK);
	if (connect(servSock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) >= 0)
	{
		if (send(servSock, &m, MSGSZ , 0) != MSGSZ)
			fatal("send() sent a different number of bytes than expected");
		for(i=0;i<MAX_ALLOWED_MEMORY_SEGMENTS;i++)
		{
			recv(servSock,&rshminfo_array[i],sizeof(rshminfo),0);
			if(rshminfo_array[i].rshmid != -1)
			{
				rshminfo_array[i].shmid = shmget(rshminfo_array[i].key,rshminfo_array[i].size,IPC_CREAT|0666);
				rshminfo_array[i].addr = shmat(rshminfo_array[i].shmid,NULL,0);
				recv(servSock,rshminfo_array[i].addr,rshminfo_array[i].size,0);
			}
		}
	}
	close(servSock);
}

int main(int argc, char *argv[])
{
	setup(argc,argv);
	for(;;)
	{
		if(msgrcv(msgqid[0], &m[0], MSGSZ, 0,IPC_NOWAIT) != -1)
		{
			switch(m[0].cmd)
			{
				case CONSTANT_RSHMGET_CHAR:
					serv_rshmget();
					break;
				case CONSTANT_RSHMAT_CHAR:
					serv_rshmat();
					break;
				case CONSTANT_RSHMDT_CHAR:
					serv_rshmdt();
					break;
				case CONSTANT_RSHMCTL_CHAR:
					serv_rshmctl();
					break;
				case CONSTANT_RSHMCHANGED_CHAR:
					serv_rshmchanged();
					break;
			}
		}
//		sleep(10);
		get_info_from_main_server();
	}
}

