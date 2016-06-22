#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/mman.h>
#include "bufferStruct.h"
#include <sys/wait.h>

SharedMem *shmem_ptr; /* pointer to struct shared segment */

/* shared memory function*/
void sharingMemory(int memKey, int memSize, int memFlag) {
    int shmem_id = shmget(memKey, memSize, memFlag); /* shared memory identifier */
    if (shmem_id == -1) {
        perror("shmget fail");
        exit(1);
    }
    shmem_ptr = shmat(shmem_id, (void *) NULL, 1023);
    if (shmem_ptr == (void *) - 1) {
        perror("shmat fail");
        exit(2);
    }
}

int main(int argc, char* argv[]) {
    pid_t child_id;

    //shared memory

    key_t memKey = 6655; /* A key to access shared memory segments */
    int memSize = sizeof(SharedMem);
    int memFlag = 1023; /* Call permissions and modes are set. */
    sharingMemory(memKey, memSize, memFlag);

    /*shared mutex*/
    pthread_mutexattr_t attr1, attr2;
    pthread_mutexattr_setpshared (&attr1, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setpshared (&attr2, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init (&shmem_ptr->lock, &attr1);
    pthread_mutex_init (&shmem_ptr->lock2, &attr2);

    /*shared conditions*/
    pthread_condattr_t spaceAttr, itemAttr;
    pthread_condattr_setpshared(&spaceAttr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&itemAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shmem_ptr->SpaceAvailable, &spaceAttr);
    pthread_cond_init(&shmem_ptr->ItemAvailable, &itemAttr);

    //split code

    if ((child_id = fork()) == -1) {
        printf("Could not fork()\n");
        exit(1);
    }
    if (child_id != 0) { //consumer
      pid_t child2_id;
      if ((child2_id = fork()) == -1) {
          printf("Could not fork()\n");
          exit(1);
      }
      if(child2_id != 0){ //consumer
          char keystr[10];
          sprintf(keystr,"%d", memKey);
          execl("./consum", "consum", keystr, NULL);
          /* done with the program, so detach the shared segment and terminate */
          shmdt ( (void *)  shmem_ptr);
          printf("Finished execution \n");
      }else{ //pblack
          char keystr[10];
          sprintf(keystr,"%d", memKey);
          execl("./pBlack", "pBlack", keystr, NULL);
      }
    }else{ //pgreen
        char keystr[10];
        sprintf(keystr,"%d", memKey);
        execl("./pGreen", "pGreen", keystr, NULL);
    }


    return 0;
}
