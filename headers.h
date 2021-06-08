#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include<string.h>
#include "list.h"

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

///==============================
//don't mess with this variable//
int *shmaddr; //
//===============================
char shmProcessKey = 'S';


int getClk()
{
    return *shmaddr;
}

/*
 * All processes call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait!ll The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All processes call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}


void compileAndRun( char* filepath, char *fileName, char *arg1, char *arg2 , char* arg3)
{
    char *compile;
    compile = (char *)malloc((20 + 2 * sizeof(fileName) * sizeof(char)));
    strcpy(compile, "gcc ");
    strcat(compile, fileName);
    strcat(compile, ".c -o ");
    strcat(compile, fileName);
    strcat(compile, ".out");

    system(compile);
    execl(filepath, arg1, arg2, arg3 , NULL);
}


int init_msgq(key_t key){
    int msgq_id = msgget(key ,  0666|IPC_CREAT);
    if(msgq_id == -1)
        printf("error in creating or retrieving a message queue\n");
    return msgq_id;
}

int initShm (key_t key , int size){
    int shmid = shmget(key , size , 0666 | IPC_CREAT);
    if(shmid == -1)
        printf("error in creating or retrieving a shared memory\n");
    return shmid;
}

void sendMessage(int msgq_id , struct process* ptr){
    int snd_value = msgsnd(msgq_id , ptr , sizeof(*ptr) , !IPC_NOWAIT);
    if(snd_value == -1)
        printf("error in sending a process with id=%d\n" , ptr->id);

}

struct process* receiveMessage(int msgq_id){
    struct process* ptr = (struct process*) malloc(sizeof(struct process));
    int rcv_value = msgrcv(msgq_id , ptr , sizeof(*ptr) , 0 , !IPC_NOWAIT);
    if(rcv_value == -1){
        printf("error in receving from a message queue\n");
        return NULL;
    }
    return ptr;

}

struct process* receiveMessage_NOWAIT(int msgq_id){
    struct process* ptr = (struct process*) malloc(sizeof(struct process));
    int rcv_value = msgrcv(msgq_id , ptr , sizeof(*ptr) , 0 , IPC_NOWAIT);
    if(rcv_value == -1){
        printf("Not receving from a message queue\n");
        return NULL;
    }
    return ptr;

}

void* shmAttach(int shmid){
    void* shm_addr = shmat(shmid , (void*)0 , 0);
    if(*(int*)shm_addr == -1)
        printf("error in attaching a shared memory\n");
    return shm_addr;
}


