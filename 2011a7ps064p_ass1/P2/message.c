#include <stdio.h>
#include <stdlib.h>
void fatal(char* message)
{
	perror(message);
	exit(1);
}