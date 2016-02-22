/* 

multicast.c

The following program sends or receives multicast packets. If invoked
with one argument, it sends a packet containing the current time to an
arbitrarily chosen multicast group and UDP port. If invoked with no
arguments, it receives and prints these packets. Start it as a sender on
just one host and as a receiver on all the other hosts

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>

#define EXAMPLE_PORT 6000
#define EXAMPLE_GROUP "239.0.0.1"
int timeout = 0;
   struct sockaddr_in addr;
   int addrlen, sock, cnt;
   struct ip_mreq mreq;
   char message[50];
   pid_t pid;
void alarm_handler(int signo)
{
  timeout = 1;
}
void sigint_handler(int signo)
{
      time_t t = time(0);
   sprintf(message, "bye-%-24.24s", ctime(&t));
      cnt = sendto(sock, message, sizeof(message), 0,
          (struct sockaddr *) &addr, addrlen);
   if (cnt < 0) {
      perror("sendto");
      kill(pid,SIGINT);
      exit(1);
   }
       
       else
        exit(0);
}
main(int argc, char* argv[])
{


   /* set up socket */
   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) {
     perror("socket");
     exit(1);
   }
   bzero((char *)&addr, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port = htons(EXAMPLE_PORT);
   addrlen = sizeof(addr);
   pid = fork();
   if (pid > 0) {
    signal(SIGINT,&sigint_handler);
      /* send */
      addr.sin_addr.s_addr = inet_addr(EXAMPLE_GROUP);
      while (1) {
	 time_t t = time(0);
	 sprintf(message, "hello-%-24.24s", ctime(&t));
	 printf("sending: %s\n", message);
	 cnt = sendto(sock, message, sizeof(message), 0,
		      (struct sockaddr *) &addr, addrlen);
	 if (cnt < 0) {
 	    perror("sendto");
	    exit(1);
	 }
	 sleep(15);
      }
   } else {
signal(SIGALRM,&alarm_handler);


      /* receive */
    int reuse = 1;
if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *)&reuse, sizeof(reuse)) < 0)
{
perror("Setting SO_REUSEPORT error");
close(sock);
exit(1);
}
      if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {        
         perror("bind");
	 exit(1);
      }    
      mreq.imr_multiaddr.s_addr = inet_addr(EXAMPLE_GROUP);         
      mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
      if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		     &mreq, sizeof(mreq)) < 0) {
	 perror("setsockopt mreq");
	 exit(1);
      }         
      while (1) {
        timeout = 0;
  alarm(5);
  int count = 0;
  while(timeout == 0)
  {

   cnt = recvfrom(sock, message, sizeof(message), 0, 
      (struct sockaddr *) &addr, &addrlen);

   if (cnt < 0) {
      perror("recvfrom");
      exit(1);
   } else if (cnt == 0) {
      break;
   } 
   else
   {
       printf("%s: message = \"%s\"\n", inet_ntoa(addr.sin_addr), message);
       if(message[0] == 'h')
       {
        message[0] = '$';
         cnt = sendto(sock, message, sizeof(message), 0,
          (struct sockaddr *) &addr, addrlen);
   if (cnt < 0) {
      perror("sendto");
      exit(1);
   }
       }
       else if(message[0] == '$')
        count++;
   }
    
  }
 	 printf("count = %d\n",count);
	 
   sleep(15);
        }
    }
}
