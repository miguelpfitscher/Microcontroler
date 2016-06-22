
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define bufferSize 4

char Buffer1[bufferSize][20];
char Buffer2[bufferSize][20];
int in1 = 0;
int in2 = 0;
int count1 = 0;
int count2 = 0;
int end = 2;

pthread_mutex_t lock, lock2;
pthread_cond_t SpaceAvailable, ItemAvailable;

void copytoBuffers(char writing[]){

  if (count1 == bufferSize) { //if count1 == bufferSize count2 != bufferSize
        strcpy (Buffer2[in2], writing);
        in2 = (in2 + 1) % bufferSize;
        count2++; /* Increment the count of items in the buffer */

  } else {
        strcpy (Buffer1[in1], writing);
        in1 = (in1 + 1) % bufferSize;
        count1++; /* Increment the count of items in the buffer */
  }

}

void * pBlack(void *arg) {
    int file, i;
    struct timeval tv;
    struct timespec tim, tim2;
    char writing[12];
    char Stringtime[4];
    if ((file = open("prod_black.txt", O_WRONLY)) <= -1) //write only
        exit(1);
    for(i = 0 ; i < 1000 ; i++){
        pthread_mutex_lock(&lock); /* Enter critical section  */

        while (count1 == bufferSize && count2 == bufferSize) /* Make sure that both buffers are NOT full   */
            while (pthread_cond_wait(&SpaceAvailable, &lock) != 0); /* Sleep using a cond variable */

        gettimeofday(&tv, NULL); //read current time
        sprintf(writing, "Black %d \n", tv.tv_usec);
        tim.tv_sec = 0;
        tim.tv_nsec = (rand() % 101) * 100;
        nanosleep(&tim, &tim2);  //a random delay before producing
        write(file, writing, strlen(writing));
        copytoBuffers(writing);
        writing[0] = '\0';
        pthread_mutex_unlock(&lock);
        pthread_cond_signal(&ItemAvailable); /* Wakeup consumer, if waiting */
    }
    pthread_mutex_lock(&lock2);
    end--;
    pthread_mutex_unlock(&lock2);
    close(file); //close the black file
}

void * pGreen(void *arg) {
    int file, i;
    struct timeval tv;
    struct timespec tim, tim2;
    char writing[12];
    char Stringtime[4];
    if ((file = open("prod_green.txt", O_WRONLY)) <= -1) //write only
        exit(1);

    for(i = 0; i < 1000; i++){
        pthread_mutex_lock(&lock); /* Enter critical section  */
        while (count1 == bufferSize && count2 == bufferSize) /* Make sure that buffer is NOT full   */
            while (pthread_cond_wait(&SpaceAvailable, &lock) != 0); /* Sleep using a cond variable */

        gettimeofday(&tv, NULL); //read current time
        sprintf(writing, "Green %d \n", tv.tv_usec);
        tim.tv_sec = 0;
        tim.tv_nsec = (rand() % 101) * 100;
        nanosleep(&tim, &tim2);  //a random delay before producing
        write(file, writing, strlen(writing));
        copytoBuffers(writing);
        writing[0] = '\0';
        pthread_mutex_unlock(&lock);
        pthread_cond_signal(&ItemAvailable); /* Wakeup consumer, if waiting */
    }

    pthread_mutex_lock(&lock2);
    end--;
    pthread_mutex_unlock(&lock2);
    close(file); //close the green file
}

void * consumer(void *arg) {
    int file;
    int out1 = 0;
    int out2 = 0;
    if ((file = open("output.txt", O_WRONLY)) <= -1) //write only
            exit(1);
    int i = -1;
    do {

        pthread_mutex_lock(&lock); /* Enter critical section  */
        while (count1 == 0 && count2 == 0) /* there is nothing to read yet */
            while (pthread_cond_wait(&ItemAvailable, &lock) != 0); /* Sleep using a cond variable */

        /* count must be > 0    */
        if (count1 > 0) {
            write(file, Buffer1[out1], strlen(Buffer1[out1]));
            out1 = (out1 + 1) % bufferSize;
            count1--; /* Decrement the count of items in the buffer */
        } else if (count2 > 0) {
            write(file, Buffer2[out2], strlen(Buffer2[out2]));
            out2 = (out2 + 1) % bufferSize;
            count2--; /* Decrement the count of items in the buffer */
        }
        i = count1 + count2; //i = 0 if count1 and count2 = 0
        pthread_mutex_unlock(&lock); /* exit critical seciton  */
        pthread_cond_signal(&SpaceAvailable); /* Wakeup producers, if waiting */
        pthread_mutex_lock(&lock2);
        if(i == 0)
            i = end; //if end = 0 prod black and prod green are finished
        pthread_mutex_unlock(&lock2);
    } while (i != 0); //while prod black and prod green are not finished and there is nothing in the buffers
    close(file); //close the output file
}

int main(int argc, char* argv[]) {
    pthread_t pG, cons, pB; /* thread variables */
    pthread_attr_t attr; /*attribute object*/
    int n;
    srand(time(NULL)); //for random time sleep
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&SpaceAvailable, NULL);
    pthread_cond_init(&ItemAvailable, NULL);

    /*  Create producer black thread                        */
    if (n = pthread_create(&pB, NULL, pBlack, NULL)) {
        fprintf(stderr, "pthread_create :%s\n", strerror(n));
        exit(1);
    }

    /*  Create producer green thread                        */
    if (n = pthread_create(&pG, NULL, pGreen, NULL)) {
        fprintf(stderr, "pthread_create :%s\n", strerror(n));
        exit(1);
    }

    /*  Create consumer thread                        */
    if (n = pthread_create(&cons, NULL, consumer, NULL)) {
        fprintf(stderr, "pthread_create :%s\n", strerror(n));
        exit(1);
    }

    /* Wait for the consumer thread to finish.         */
    if (n = pthread_join(cons, NULL)) {
        fprintf(stderr, "pthread_join:%s\n", strerror(n));
        exit(1);
    }

    printf("Finished execution \n");
    return 0;
}
