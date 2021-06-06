#include "headers.h"


#define Pkey 66


/* Modify this file as needed*/
int remainingtime;


// the algorithm of fisrt come first servred
void FCFS_OR_SJF(); 
int msgq_process;


// some needed variables
int current_time;


int main(int agrc, char *argv[])
{
    initClk();

    char* algo = argv[1];
    if(strcmp(algo , "fcfs")==0 || strcmp(algo , "sjf")==0)
        FCFS_OR_SJF();
    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    

    destroyClk(false);

    return 0;
}



void  FCFS_OR_SJF(){

    key_t key = ftok("/home/ahmed/Desktop" , Pkey);
    msgq_process = init_msgq(key);
    current_time = getClk();
    struct process* ptr = receiveMessage(msgq_process);
    if(ptr != NULL){
        printf("process %d arrived at time %d remaining %d\n" , ptr->id , getClk() , ptr->runtime);
        remainingtime = ptr->runtime;
        while (remainingtime > 0)
        {
            if(current_time != getClk()){
                remainingtime--;
                current_time = getClk();
            }
                
            
        }
        printf("process %d finished at time %d remaining %d\n" , ptr->id , getClk() , 0);
        //free(ptr);
        
    }
}
