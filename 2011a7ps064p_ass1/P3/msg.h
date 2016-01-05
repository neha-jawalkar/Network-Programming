typedef struct msg{
	long type;
	char cmd;
	int rshmkey;
	int rshmid;
	int rshmsize;
	int retval;
}msg;

#define MSGSZ sizeof(msg)