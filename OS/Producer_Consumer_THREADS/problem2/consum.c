
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <string.h>
#include "bufferStruct.h"

int main(int argc, char* argv[]) {

  int out1 = 0;
  int out2 = 0;
  /*shared memory code*/
  int id;         /* shared memory identifier */
  SharedMem *ptr;       /* pointer to shared memory */
  id = shmget (atoi(argv[1]), 0, 0);
  if (id == -1){
      perror ("child shmget failed");
      exit (1);
  }
  ptr = shmat (id, (void *) NULL, 1023);
  if (ptr == (void *) -1){
      perror ("child shmat failed");
      exit (2);
  }

  /*opening file*/
  int file;
  if ((file = open("output.txt", O_WRONLY)) <= -1) //write only
          exit(1);
  int i = -1;
  do {

    pthread_mutex_lock(&ptr->lock); /* Enter critical section  */

    while (ptr->count1 == 0 && ptr->count2 == 0) /* there is nothing to read yet */
        while (pthread_cond_wait(&ptr->ItemAvailable, &ptr->lock) != 0); /* Sleep using a cond variable */

    /* count must be > 0    */
    if (ptr->count1 > 0) {
        write(file, ptr->Buffer1[out1], strlen(ptr->Buffer1[out1]));
        out1 = (out1 + 1) % bufferSize;
        ptr->count1--; /* Decrement the count of items in the buffer */
    } else if (ptr->count2 > 0) {
        write(file, ptr->Buffer2[out2], strlen(ptr->Buffer2[out2]));
        out2 = (out2 + 1) % bufferSize;
        ptr->count2--; /* Decrement the count of items in the buffer */
    }
    i = ptr->count2 + ptr->count1; //if both counts are 0, i = 0
    pthread_mutex_unlock(&ptr->lock); /* exit critical seciton  */
    pthread_cond_signal(&ptr->SpaceAvailable); /* Wakeup producers, if waiting */
    if (i == 0){
        pthread_mutex_lock(&ptr->lock2);
        i = ptr->end + 2; //if count 1 and count 2 and both producers are finished i = 0
        pthread_mutex_unlock(&ptr->lock2);
    }
  } while (i != 0); //while count1 and count2 are not 0 or the both pblack and pgreen are not finish
  close(file); //close the output file
  return 0;
}
