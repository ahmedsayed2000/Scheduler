#include "headers.h"

# define Mkey 65

int msgq_id;
int scheduler_id;
void clearResources(int);

void wait_scheduler();

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    char* process_file = argv[1];
    FILE* file = fopen(process_file , "r");
    int id , arrival , runtime , priority;
    struct list* process_list = (struct list*) malloc(sizeof(struct list));
    if(file == NULL){
        printf("error in openeing the file\n");
    }
    else{
        initList(process_list);
        fscanf(file , "%d %d %d %d" , &id , &arrival , &runtime , &priority);
        struct process * prc = (struct process*) malloc(sizeof(struct process));
        struct process * prc1 = (struct process*) malloc(sizeof(struct process));
        struct process * prc2 = (struct process*) malloc(sizeof(struct process));
        //struct process* last = (struct process*) malloc(sizeof(struct process));
        //struct process* last2 = (struct process*) malloc(sizeof(struct process));
        struct process* end = (struct process*) malloc(sizeof(struct process));
        prc->id = id;   prc->arrival=arrival;   prc->runtime = runtime;   prc->priority = priority;  prc->remaining_time=prc->runtime; prc->last_process=0;  prc->last_in_second=0;
        fscanf(file , "%d %d %d %d" , &id , &arrival , &runtime , &priority);
        prc1->id = id;   prc1->arrival=arrival;   prc1->runtime = runtime;   prc1->priority = priority;  prc1->last_process=0; prc1->remaining_time=prc1->runtime;;  prc1->last_in_second=1;
        fscanf(file , "%d %d %d %d" , &id , &arrival , &runtime , &priority);
        prc2->id = id;   prc2->arrival=arrival;   prc2->runtime = runtime;   prc2->priority = priority;   prc2->remaining_time=prc2->runtime;  prc2->last_process=0;  prc2->last_in_second=0;
        //last->id=0; last->arrival=prc1->arrival;  last->priority=-1;  last->runtime=5000;
        //last2->id=0; last2->arrival=prc2->arrival;  last2->priority=-1;  last2->runtime=5000;
        fscanf(file , "%d %d %d %d" , &id , &arrival , &runtime , &priority);
        end->id=id;  end->arrival=arrival;  end->priority=priority;  end->runtime=runtime;  end->last_process=1; end->remaining_time=end->runtime;; end->last_in_second=1;
        add(process_list , prc);
        add(process_list , prc1);
        //add(process_list , last);
        add(process_list , prc2);
        //add(process_list,last2);
        add(process_list,end);
        
        printf("addition is done\n");
        fclose(file);
    }
    printf("message queue is done\n");
    key_t key = ftok("keyfile" , Mkey);
    msgq_id = init_msgq(key);
    printf("message queue id=%d\n" , msgq_id);

    
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    char* algo = argv[2];
    char* quantom = NULL;
    if(strcmp(algo,"rr")==0)
    {
        quantom = argv[3];
       // printf("qhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhq=%s\n",quantom);
        //printf("arggggggggggggg=%s\n",argv[3]);
        
    }

    // 3. Initiate and create the scheduler and clock processes.
    int pid;
    pid = fork();
    if(pid==-1)
        perror("error in forking\n");
    else if(pid==0){
        printf("Iam the clock and my pid = %d\n" , getpid());
        compileAndRun("clk.out" , "clk" , "clk.out" , NULL , NULL);
    }
    else{
            // forking the sheduler
            pid = fork();
            if(pid == 0){
                printf("Iam the scheduler and my pid = %d\n" , getpid());
                //scheduler_id = getpid();
                compileAndRun("scheduler.out" , "scheduler" , "scheduler.out" , algo , quantom);
            }
            else if(pid == -1){
                printf("error in forking the scheduler\n");
            }
            else{

                // 4. Use this function after creating the clock process to initialize clock.
                initClk();
                scheduler_id = pid;      
                printf("Iam the process generator and my pid = %d\n" , getpid());
                printf("clock initialised at the process generator\n");
            }
            
    
    }


    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
     while(isEmpty(process_list)==0){
        struct process* ptr = search(process_list , x);
            while(ptr!=NULL){
                sendMessage(msgq_id , ptr);
                printf("process arrived at time=%d\n" , getClk());
                clear(process_list , ptr);
                ptr = search(process_list , x);
            }
        
        
        x= getClk();
        //printf("Current Time is %d\n", x);

    }
    
    wait_scheduler();
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(false);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    //kill(0 , SIGKILL);
    printf("generator interrupted\n");
    msgctl(msgq_id , IPC_RMID , (struct msqid_ds*) 0);

}

void wait_scheduler(){
    /*int id;
    while(1){
        int stat_loc;
        id=wait(&stat_loc);
        if(id==scheduler_id)
            break;
    }*/
    int stat_loc;
    waitpid(scheduler_id , &stat_loc , 0);
}