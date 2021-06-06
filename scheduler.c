#include "headers.h"

#define Mkey 65
#define Pkey 66



// this id the message queue variables SCHEDULER  <------------> PROCESS_GENEARTOR
int msgq_id;
struct list* p_queue;

// this id the message queue variables SCHEDULER  <------------> PROCESS
int msgq_process;




// handler function
void handler(int signum);

// varaibles used to write in output files
FILE* sch_log;
FILE* sch_perf;
void open_files();
void close_files();


// some needed functions
void FCFS(char*);
void SJF(char*);
float avg_wait_time;
int wait_time_sum;
int wait_idle;
int process_num;
float WTA_sum;
float avg_WTA;
float CPU_util;
int total_running;
int current_time;
struct list* temp;
void calculate();

int main(int argc, char *argv[])
{
    signal(SIGINT , handler);
    initClk();
    char* algo = argv[1];
    open_files();
    if(strcmp(algo , "fcfs")== 0)
        FCFS(algo);
    else if(strcmp(algo , "sjf")==0)
        SJF(algo);
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}

void FCFS(char* algo){
    p_queue = (struct list*) malloc(sizeof(struct list));
    initList(p_queue);
    key_t key1 = ftok("/home/ahmed/Desktop" , Mkey);
    msgq_id = init_msgq(key1);
    key_t key3 = ftok("/home/ahmed/Desktop" , Pkey);
    msgq_process = init_msgq(key3);

    wait_time_sum=0;
    process_num =0;
    WTA_sum = 0;
    total_running=0;
    current_time = getClk();

    struct process* ptr ;
    
    while((ptr=receiveMessage(msgq_id))!=NULL){
        if(ptr->id == -1)
            break;
        while(ptr->id != 0){
            add(p_queue , ptr);
            ptr = receiveMessage(msgq_id);
        }
        while(isEmpty(p_queue)==0){
            struct process* prc = get_first(p_queue);
            if(prc != NULL){
                wait_time_sum = wait_time_sum + getClk() - prc->arrival;
                total_running = total_running + prc->runtime;
                process_num++;
                int pid;
                pid = fork();
                if(pid==-1)
                    printf("error in forking\n");
                else if(pid==0){
                    printf("a process forked from the scheduler with id=%d\n" ,prc->id );
                    compileAndRun("/home/ahmed/Desktop/project/process.out" , "process" , "process.out" ,algo  , NULL);
                }
                else{
                    int waited = getClk() - prc->arrival;
                    fprintf(sch_log , "# At time %d process %d %s arr %d total %d remain %d wait %d\n" , getClk() , prc->id , "started" , prc->arrival , prc->runtime , prc->runtime , getClk()-prc->arrival);
                    sendMessage(msgq_process , prc);
                    int stat_loc;
                    wait(&stat_loc);
                    fprintf(sch_log , "# At time %d process %d %s arr %d total %d remain %d wait %d  TA %d  WTA %f\n" , getClk() , prc->id , "finished" , prc->arrival , prc->runtime , 0, waited , getClk()-prc->arrival , (getClk()-prc->arrival)/(float)prc->runtime);
                    WTA_sum = WTA_sum + (float)(getClk()-prc->arrival)/prc->runtime;
                    free(prc);
                }
            
            }
        }
        
        
    }
    printf("got out of the while loop\n");
    calculate();
}

void SJF(char* algo){
    p_queue = (struct list*) malloc(sizeof(struct list));
    initList(p_queue);
    key_t key1 = ftok("/home/ahmed/Desktop" , Mkey);
    msgq_id = init_msgq(key1);
    key_t key3 = ftok("/home/ahmed/Desktop" , Pkey);
    msgq_process = init_msgq(key3);
    

    wait_time_sum=0;
    process_num =0;
    WTA_sum = 0;
    total_running=0;
    current_time = getClk();
    struct process* ptr ;
    int counter =0;
    int x=0;
    bool check = false;
    while((ptr=receiveMessage(msgq_id))!=NULL){
        if(ptr->id == -1 && isEmpty(p_queue)==1)
            break;
        else if(ptr->id == -1 && isEmpty(p_queue)==0){
            counter=p_queue->count;
            check=true;
        }
        else{
            counter=1;
        }
        while(ptr->id != 0 && ptr->id != -1){
            printf("process received with id=%d\n" , ptr->id);
            add_sjf(p_queue , ptr);
            ptr = receiveMessage(msgq_id);
        }
        while(x<counter){
            struct process* prc = get_first(p_queue);
            if(prc != NULL){
                wait_time_sum = wait_time_sum + getClk() - prc->arrival;
                total_running = total_running + prc->runtime;
                process_num++;
                int pid;
                pid = fork();
                if(pid==-1)
                    printf("error in forking\n");
                else if(pid==0){
                    printf("a process forked from the scheduler with id=%d\n" ,prc->id );
                    compileAndRun("/home/ahmed/Desktop/project/process.out" , "process" , "process.out" ,algo  , NULL);
                }
                else{
                    int waited = getClk() - prc->arrival;
                    fprintf(sch_log , "# At time %d process %d %s arr %d total %d remain %d wait %d\n" , getClk() , prc->id , "started" , prc->arrival , prc->runtime , prc->runtime , getClk()-prc->arrival);
                    sendMessage(msgq_process , prc);
                    int stat_loc;
                    wait(&stat_loc);
                    fprintf(sch_log , "# At time %d process %d %s arr %d total %d remain %d wait %d  TA %d  WTA  %f\n" , getClk() , prc->id , "finished" , prc->arrival , prc->runtime , 0, waited , getClk()-prc->arrival , (getClk()-prc->arrival)/(float) prc->runtime);
                    WTA_sum = WTA_sum + (getClk()-prc->arrival)/(float)prc->runtime;
                    free(prc);
                }
            }
            x++;
        }
        x=0;
        if(check == true)
            break;
        
        
    }
    calculate();

}

void handler(int signum){
    // in this function we clear the resources
    msgctl(msgq_process , IPC_RMID , (struct msqid_ds*) 0);
    close_files();
}

void open_files(){
    sch_log = fopen("scheduler.log" , "w");
    fprintf(sch_log , "# At time x process y state arr w total z remacleain y wait k\n");
    sch_perf = fopen("scheduler.perf" , "w");
}

void close_files(){
    fclose(sch_log);
    fclose(sch_perf);
}

void calculate(){
    avg_wait_time=0;
    avg_WTA=0;
    CPU_util = 0;

    avg_wait_time = (float)wait_time_sum / process_num;
    CPU_util = (float)total_running / getClk() *100;
    avg_WTA = WTA_sum/process_num;
    fprintf(sch_perf , "CPU utilisation=%f\n" , CPU_util);
    fprintf(sch_perf , "Avg WTA= %f\n" , avg_WTA);
    fprintf(sch_perf , "Avg waiting= %f\n" , avg_wait_time);
    
}