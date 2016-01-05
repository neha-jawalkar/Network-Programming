#include <sys/types.h>
#define CONSTANT_MESSAGE_FROM_REMOTE_ENDPOINT "5"
#define CONSTANT_RSHMGET_CHAR '0'
#define CONSTANT_RSHMAT_CHAR '1'
#define CONSTANT_RSHMDT_CHAR '2'
#define CONSTANT_RSHMCTL_CHAR '3'
#define CONSTANT_RSHMCHANGED_CHAR '4'
#define CONSTANT_MESSAGE_FROM_REMOTE_ENDPOINT_CHAR '5'
#define SEND_ERROR "send() sent a different number of bytes than expected"
#define RCV_ERROR "recv() failed or connection closed prematurely"
#define MAX_REMOTE_ENDPOINTS 10
#define MAX_ALLOWED_MEMORY_SEGMENTS 100
#define MAXPENDING 5
#define RSHMINFOSZ sizeof(rshminfo)

typedef struct rshminfo{
int rshmid;
/*unique id across all systems. created by the
first system*/
key_t key; /*key used to create shm segment*/
int shmid; /*shmid returned by the local system*/
void *addr; /*address returned by the local system*/
size_t size;
int rm;
int ref_count; /*no of processes attached to*/

}rshminfo;

#define MSGSZ sizeof(msg)
