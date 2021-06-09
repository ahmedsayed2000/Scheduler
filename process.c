#include "headers.h"

#define Pkey 66

/* Modify this file as needed*/
int remainingtime;
int shmid;
int *RM;

// the algorithm of fisrt come first servred
void FCFS_OR_SJF();
int msgq_process;
int quantom;

// some needed variables
int current_time;

int main(int agrc, char *argv[])
{
    initClk();

    char *algo = argv[1];
    if (strcmp(algo, "1") == 0 || strcmp(algo, "2") == 0)
        FCFS_OR_SJF();
    else if (strcmp(algo, "5") == 0 || strcmp(algo , "4")==0 || strcmp(algo , "3")==0)
    {
        

        int check = -1;
        shmid = initShm(shmProcessKey, 4);
        RM = (int *)shmAttach(shmid);

        while (*RM > 0)
        {
            while (check == getClk())
                     ;
            if (check != getClk())
            {
                (*RM)--;
                check = getClk();
            }
        }
    }

    
    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;

    destroyClk(false);

    return 0;
}

void FCFS_OR_SJF()
{

    key_t key = ftok("keyfile", Pkey);
    msgq_process = init_msgq(key);
    current_time = getClk();
    struct process *ptr = receiveMessage(msgq_process);
    if (ptr != NULL)
    {
        printf("process %d arrived at time %d remaining %d\n", ptr->id, getClk(), ptr->runtime);
        remainingtime = ptr->runtime;
        while (remainingtime > 0)
        {
            if (current_time != getClk())
            {
                remainingtime--;
                current_time = getClk();
            }
        }
        printf("process %d finished at time %d remaining %d\n", ptr->id, getClk(), 0);
        //free(ptr);
    }
}
