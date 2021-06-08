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
        size_t len = 0;
        char *line = NULL;
        int numberOfProcesses = 0;
        //getline(&line, &len, file);
        while (getline(&line, &len, file) != -1)
             if (line[0] != '#')
                numberOfProcesses++;
        printf("process num is %d\n" , numberOfProcesses);

        fclose(file);
        file = fopen(process_file , "r");
        int id,arrival,runtime,priority;
        getline(&line, &len, file);
        fscanf(file , "%d\t%d\t%d\t%d" , &id , &arrival , &runtime , &priority);
        struct process* last=(struct process*)malloc(sizeof(struct process));
        last->arrival=arrival;  last->id=id;  last->priority=priority; last->runtime=runtime;  last->remaining_time=runtime;  last->last_process=0;  last->last_in_second=0;
        int last_second=last->arrival;
        add(process_list , last);
        int temp=0;
        while (temp<numberOfProcesses-1)
        {
            fscanf(file , "%d\t%d\t%d\t%d" , &id , &arrival , &runtime , &priority);

            struct process* ptr = (struct process*)malloc(sizeof(struct process));
            ptr->id = id;  ptr->arrival=arrival;  ptr->runtime=runtime;  ptr->priority=priority; ptr->remaining_time=runtime;  ptr->last_process=0;  ptr->last_in_second=0;
            add(process_list , ptr);
            if(arrival != last_second){
                last->last_in_second=1;
            }
            last = ptr;
            last_second = last->arrival;
            temp++;
        }
        fclose(file);
        last->last_process=1;
        last->last_in_second=1;
        printf("last process id=%d\n" , last->id);
        display(process_list);
    }
    printf("message queue is done\n");
    key_t key = ftok("keyfile" , Mkey);
    msgq_id = init_msgq(key);
    printf("message queue id=%d\n" , msgq_id);

    
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    char* algo = argv[2];
    char* quantom = NULL;
    if(strcmp(algo,"5")==0)
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
