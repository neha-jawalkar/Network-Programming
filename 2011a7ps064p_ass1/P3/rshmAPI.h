#include <sys/types.h>
void setup(char* argv[]);
int rshmget(key_t key, size_t size);
void* rshmat(int rshmid, void* addr);
int rshmdt(int rshmid, void* addr);
int rshmctl(int rshmid, int cmd);
void rshmChanged(int rshmid);