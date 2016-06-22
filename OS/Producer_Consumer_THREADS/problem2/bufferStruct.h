#include <pthread.h>
#define bufferSize 4

typedef struct sharedMem { //shared memory struct
    char Buffer1[bufferSize][20]; // 4*20 = 80 bytes
    char Buffer2[bufferSize][20]; // 4*20 = 80 bytes
    int count1;
    int count2;
    int end;
    int in1;
    int in2;
    pthread_mutex_t lock;
    pthread_mutex_t lock2;
    pthread_cond_t SpaceAvailable;
    pthread_cond_t ItemAvailable;
} SharedMem;
