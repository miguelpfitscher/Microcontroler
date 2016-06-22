#define _REENTRANT

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>

/*  shared memory code */

typedef struct sharedMem { //shared memory struct
    char buffer[1024]; // 1024 bytes
    int timestamp;
} SharedMem;

SharedMem *shmem_ptr; /* pointer to struct shared segment */
int file = 0;

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

int size = 0; //global variable used for consumer handle be capable to write.

/* function that I got from realtime.c file in CSCI 5103 site */
void send_rt_signal(int signo, int value, pid_t child_id) {
    union sigval sivalue;
    sivalue.sival_int = value;
    /* queue the signal */
    if (sigqueue(child_id, signo, sivalue) < 0) {
        fprintf(stderr, "sigqueue failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return;
}

/* consumer handler*/
void consumer(int signal, siginfo_t *info, void *arg __attribute__((__unused__))) {
    size = info->si_value.sival_int;

    if (size != -1) {
        write(file, shmem_ptr->buffer, size);
    }
    send_rt_signal(SIGRTMIN + 1, 0, getppid()); //send a real time signal to producer process  with 0
    return;
}

/*producer handler*/
void producer() { //nothing to do here.
    return;
}

int main(int argc, char *argv[]) {

    key_t memKey = 6666; /* A key to access shared memory segments */
    int memSize = 1028; // 1024 buffer bytes + 4 timestamp bytes
    int memFlag = 1023; /* Call permissions and modes are set. */
    sharingMemory(memKey, memSize, memFlag);


    // signal variables
    sigset_t wait_set;
    struct sigaction action;

    //time variable
    struct timeval tv;

    //__________________________________________________________//
    pid_t child_id;
    int stat;

    if ((child_id = fork()) == -1) {
        printf("Could not fork()\n");
        exit(-1);
    }
    //____________________________________________________________//

    if (child_id != 0) { //producer
        if ((file = open(argv[1], O_RDONLY)) <= -1) { //read only
            send_rt_signal(SIGRTMIN + 1, -1, child_id); //send a signal to end the consumer process.
            return 1;
        }

        action.sa_sigaction = producer; //set the producer handle
        action.sa_flags = SA_SIGINFO;
        sigaction(SIGRTMIN + 1, &action, NULL);
        int r = read(file, shmem_ptr->buffer, 1024); //read file and write it into shared memory;
        gettimeofday(&tv, NULL);
        shmem_ptr->timestamp = tv.tv_usec; //write timestamp into shared memory
		while (r != 0) {
            send_rt_signal(SIGRTMIN + 1, strlen(shmem_ptr->buffer), child_id); //send real time to consumer
            sigemptyset(&wait_set);
            sigsuspend(&wait_set); //suspend execution until a real time signal is received
            r = read(file, shmem_ptr->buffer, 1024); //read file and write it into shared memory
        }
        send_rt_signal(SIGRTMIN + 1, -1, child_id); //send real time to consumer//sent a real time to consumer with -1
        wait(&stat); //wait for consumer
        shmdt((void *) shmem_ptr); //detach the shared memory
        close(file);
    } else {
        sigemptyset(&wait_set);
        sigaddset(&wait_set, SIGRTMIN + 1);
        sigprocmask(SIG_BLOCK, &wait_set, NULL); //block signal
        int relTime = 0;
        if ((file = open("output.txt", O_WRONLY)) <= -1) //write only
            return 1;
        action.sa_sigaction = consumer;
        action.sa_flags = SA_SIGINFO;
        sigaction(SIGRTMIN + 1, &action, NULL);
        sigprocmask(SIG_UNBLOCK, &wait_set, NULL); //unblock signal
        while (1) {
            sigemptyset(&wait_set);
            sigsuspend(&wait_set); //suspend execution until a real time signal is received
            if (size == -1) {
                break;
            }
        }
        gettimeofday(&tv, NULL); //read current time
        relTime = (tv.tv_usec - shmem_ptr->timestamp); //compute the diference between A and B and agregate.
        close(file); //close the output file
        printf("time for download: %d \n", relTime); //report the time for download and exit.
    }
    return 0;
}
