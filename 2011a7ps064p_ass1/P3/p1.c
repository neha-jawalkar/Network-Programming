#include "rshmAPI.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[])
{
	setup(argv);
	int a = rshmget(atoi(argv[5]),atoi(argv[6]));
	printf("RSHMID : %d\n",a);
	char* b = (char*)rshmat(a,(void*)NULL);
	printf("SHMCTL : %d\n",rshmctl(a,1));
	b[0] = '1';
	b[1] = '2';
	b[2] = '\0';
	rshmChanged(a);
	printf("CONTENTS OF SHARED MEMORY : %s\n", b);
	return 0;
}