
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "bufferStruct.h"

pthread_mutex_t lock2;

int main(int argc, char* argv[]) {

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

    /* FILE logic*/
    int file, i;
    //srand(time(NULL)); //for random time sleep
    struct timeval tv;
    struct timespec tim, tim2;
    char Stringtime[4], writing[12];
    if ((file = open("prod_black.txt", O_WRONLY)) <= -1) //write only
        exit(1);

    for(i = 0; i < 1000; i++){

        pthread_mutex_lock(&ptr->lock); /* Enter critical section  */
        while (ptr->count1 == bufferSize && ptr->count2 == bufferSize) /* Make sure that both buffers are NOT full   */
            while (pthread_cond_wait(&ptr->SpaceAvailable, &ptr->lock) != 0); /* Sleep using a cond variable */

        gettimeofday(&tv, NULL); //read current time
        sprintf(writing, "Black %d \n", tv.tv_usec);
        tim.tv_sec = 0;
        tim.tv_nsec = (rand() % 101) * 100;
        nanosleep(&tim, &tim2);  //a random delay before producing
        write(file, writing, strlen(writing));

        if (ptr->count1 == bufferSize) { //if count1 == bufferSize count2 != bufferSize
              strcpy (ptr->Buffer2[ptr->in2], writing);
              ptr->in2 = (ptr->in2 + 1) % bufferSize;
              ptr->count2++; /* Increment the count of items in the buffer */

        } else {
              strcpy (ptr->Buffer1[ptr->in1], writing);
              ptr->in1 = (ptr->in1 + 1) % bufferSize;
              ptr->count1++; /* Increment the count of items in the buffer */

        }
        writing[0] = '\0';
        pthread_mutex_unlock(&ptr->lock);
        pthread_cond_signal(&ptr->ItemAvailable); /* Wakeup consumer, if waiting */
    }
    pthread_mutex_lock(&ptr->lock2);
    ptr->end--; //variable that says if the process black is done
    pthread_mutex_unlock(&ptr->lock2);
    close(file); //close the black file
}
