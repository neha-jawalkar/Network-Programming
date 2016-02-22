#define MSGSZ sizeof(msg)

typedef struct msg{
	long type;
	int connfd;
	char state;
	int hash_index;
}msg;