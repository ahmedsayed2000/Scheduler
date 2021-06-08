#include "headers.h"

#define Mkey 65
#define Pkey 66

// this id the message queue variables SCHEDULER  <------------> PROCESS_GENEARTOR
int msgq_id;
struct list *p_queue;

// this id the message queue variables SCHEDULER  <------------> PROCESS
int msgq_process;

int shmid;
int *shmRM;
int lastprocess;

// handler function
void handler(int signum);

// varaibles used to write in output files
FILE *sch_log;
FILE *sch_perf;
void open_files();
void close_files();

// some needed functions
void FCFS(char *);
void SJF(char *);
void RR(char *, char*);
void SRTN(char *);
void HPF(char *);
float avg_wait_time;
int wait_time_sum;
int wait_idle;
int process_num;
float WTA_sum;
float avg_WTA;
float CPU_util;
int total_running;
int current_time;
char *algo;
char *quatom;
struct list *temp;
void calculate();

int main(int argc, char *argv[])
{
    signal(SIGINT, handler);
    initClk();
    algo = argv[1];
    shmid = initShm(shmProcessKey, 4);
    open_files();
    if (strcmp(algo, "fcfs") == 0)
        FCFS(algo);
    else if (strcmp(algo, "sjf") == 0)
        SJF(algo);
    else if (strcmp(algo, "rr") == 0)
    {

        quatom = argv[2];
       // printf("argv=%s\n",argv[2]);
        //printf("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq=%s\n",quatom);
        RR(algo, quatom);
    }
    else if (strcmp(algo, "srtn") == 0)
        SRTN(algo);
    else if (strcmp(algo, "hpf"))
        HPF(algo);
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}

void FCFS(char *algo)
{
    p_queue = (struct list *)malloc(sizeof(struct list));
    initList(p_queue);
    key_t key1 = ftok("keyfile", Mkey);
    msgq_id = init_msgq(key1);
    key_t key3 = ftok("keyfile", Pkey);
    msgq_process = init_msgq(key3);

    wait_time_sum = 0;
    process_num = 0;
    WTA_sum = 0;
    total_running = 0;
    current_time = getClk();
    int last = 0;

    struct process *ptr;

    while ((ptr = receiveMessage(msgq_id)) != NULL)
    {
        if (ptr->last_process == 1)
        {
            last = 1;
            add(p_queue, ptr);
        }
        else
        {
            while (1)
            {
                if (ptr->last_in_second == 1)
                {
                    if (ptr->last_process)
                        last = 1;
                    add(p_queue, ptr);
                    break;
                }
                add(p_queue, ptr);
                ptr = receiveMessage(msgq_id);
            }
        }

        while (isEmpty(p_queue) == 0)
        {
            struct process *prc = get_first(p_queue);
            if (prc != NULL)
            {
                wait_time_sum = wait_time_sum + getClk() - prc->arrival;
                total_running = total_running + prc->runtime;
                process_num++;
                int pid;
                pid = fork();
                if (pid == -1)
                    printf("error in forking\n");
                else if (pid == 0)
                {
                    printf("a process forked from the scheduler with id=%d\n", prc->id);
                    compileAndRun("process.out", "process", "process.out", algo, NULL);
                }
                else
                {
                    int waited = getClk() - prc->arrival;
                    fprintf(sch_log, "# At time %d process %d %s arr %d total %d remain %d wait %d\n", getClk(), prc->id, "started", prc->arrival, prc->runtime, prc->runtime, getClk() - prc->arrival);
                    sendMessage(msgq_process, prc);
                    int stat_loc;
                    wait(&stat_loc);
                    fprintf(sch_log, "# At time %d process %d %s arr %d total %d remain %d wait %d  TA %d  WTA %f\n", getClk(), prc->id, "finished", prc->arrival, prc->runtime, 0, waited, getClk() - prc->arrival, (getClk() - prc->arrival) / (float)prc->runtime);
                    WTA_sum = WTA_sum + (float)(getClk() - prc->arrival) / prc->runtime;
                    free(prc);
                }
            }
        }
        if (last == 1)
            break;
    }
    printf("got out of the while loop\n");
    calculate();
}

void SJF(char *algo)
{
    p_queue = (struct list *)malloc(sizeof(struct list));
    initList(p_queue);
    key_t key1 = ftok("keyfile", Mkey);
    msgq_id = init_msgq(key1);
    key_t key3 = ftok("keyfile", Pkey);
    msgq_process = init_msgq(key3);

    wait_time_sum = 0;
    process_num = 0;
    WTA_sum = 0;
    total_running = 0;
    current_time = getClk();
    struct process *ptr;
    int counter = 0;
    int last = 0;
    int x = 0;
    bool check = false;
    while ((ptr = receiveMessage(msgq_id)) != NULL)
    {
        if (ptr->id != 0)
        {
            if (ptr->last_process == 1)
            {
                add_sjf(p_queue, ptr);
                last = 1;
                counter = p_queue->count;
            }
            else
            {
                while (1)
                {
                    if (ptr->last_in_second == 1)
                    {
                        if (ptr->last_process)
                        {
                            last = 1;
                            add_sjf(p_queue, ptr);
                            counter = p_queue->count;
                            break;
                        }
                        add_sjf(p_queue, ptr);
                        counter = 1;
                        break;
                    }
                    add_sjf(p_queue, ptr);
                    ptr = receiveMessage(msgq_id);
                }
            }
        }
        /*else{
            counter=1;
        }*/
        while (ptr->id != 0 && ptr->id != -1)
        {
            printf("process received with id=%d\n", ptr->id);
            add_sjf(p_queue, ptr);
            ptr = receiveMessage(msgq_id);
        }
        while (x < counter)
        {
            struct process *prc = get_first(p_queue);
            if (prc != NULL)
            {
                wait_time_sum = wait_time_sum + getClk() - prc->arrival;
                total_running = total_running + prc->runtime;
                process_num++;
                int pid;
                pid = fork();
                if (pid == -1)
                    printf("error in forking\n");
                else if (pid == 0)
                {
                    printf("a process forked from the scheduler with id=%d\n", prc->id);
                    compileAndRun("process.out", "process", "process.out", algo, NULL);
                }
                else
                {
                    int waited = getClk() - prc->arrival;
                    fprintf(sch_log, "# At time %d process %d %s arr %d total %d remain %d wait %d\n", getClk(), prc->id, "started", prc->arrival, prc->runtime, prc->runtime, getClk() - prc->arrival);
                    sendMessage(msgq_process, prc);
                    int stat_loc;
                    wait(&stat_loc);
                    fprintf(sch_log, "# At time %d process %d %s arr %d total %d remain %d wait %d  TA %d  WTA  %f\n", getClk(), prc->id, "finished", prc->arrival, prc->runtime, 0, waited, getClk() - prc->arrival, (getClk() - prc->arrival) / (float)prc->runtime);
                    WTA_sum = WTA_sum + (getClk() - prc->arrival) / (float)prc->runtime;
                    free(prc);
                }
            }
            x++;
        }
        x = 0;
        if (check == true)
            break;
    }
    calculate();
}

void handler(int signum)
{
    // in this function we clear the resources
    msgctl(msgq_process, IPC_RMID, (struct msqid_ds *)0);
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
    close_files();
}

void open_files()
{

    sch_log = fopen("scheduler.log", "w");
    //fprintf(sch_log, "# At time x process y state arr w total z remacleain y wait k\n");
    sch_perf = fopen("scheduler.perf", "w");
}

void close_files()
{
    fclose(sch_log);
    fclose(sch_perf);
}

void calculate()
{
    avg_wait_time = 0;
    avg_WTA = 0;
    CPU_util = 0;

    avg_wait_time = (float)wait_time_sum / process_num;
    CPU_util = (float)total_running / getClk() * 100;
    avg_WTA = WTA_sum / process_num;
    fprintf(sch_perf, "CPU utilisation=%.2f\n", CPU_util);
    fprintf(sch_perf, "Avg WTA= %.2f\n", avg_WTA);
    fprintf(sch_perf, "Avg waiting= %.2f\n", avg_wait_time);
}

void WaitForSecond(int *oneSecond)
{
    while (*oneSecond == getClk())
        ;
    *oneSecond = getClk();
}

void continueProcess(struct process *prc)
{
    kill(prc->pid, SIGCONT);
    // sch_log = fopen("scheduler.log", "w");
    fprintf(sch_log, "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
            getClk(), prc->id, prc->arrival, prc->runtime,
            prc->remaining_time, getClk() - prc->arrival - (prc->runtime - prc->remaining_time));
}

void stopProcess(struct process *prc)
{
    *shmRM = -1;
    kill(prc->pid, SIGSTOP);
    //sch_log = fopen("scheduler.log", "w");
    fprintf(sch_log, "At time %d process %d stopped arr %d total %d remain %d wait %d\n",
            getClk(), prc->id, prc->arrival, prc->runtime,
            prc->remaining_time, getClk() - prc->arrival - (prc->runtime - prc->remaining_time));
}

int startProcess(struct process *prc)
{
    prc->pid = fork();

    if (prc->pid == 0) // start a new process
    {

        compileAndRun("process.out", "process", "process.out", algo, quatom);
    }

    //sch_log = fopen("scheduler.log", "w");
    fprintf(sch_log, "At time %d process %d started arr %d total %d remain %d wait %d\n",
            getClk(), prc->id, prc->arrival, prc->runtime,
            prc->remaining_time, getClk() - prc->arrival - (prc->runtime - prc->remaining_time));

    return prc->pid;
}

int finishProcess(struct process *prc)
{
    double WTA = (getClk() - prc->arrival) * 1.0 / prc->runtime;
    WTA_sum += WTA;
    int wait = (getClk() - prc->arrival) - prc->runtime;
    wait_time_sum += wait;
    process_num++;
    total_running = total_running + prc->runtime;
    // sch_log = fopen("scheduler.log", "w");
    fprintf(sch_log, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(),
            prc->id, prc->arrival, prc->runtime,
            wait, getClk() - prc->arrival, WTA);
    //clear(p_queue, prc);
    free(prc);
    *shmRM = -1;
}

void readqueue(struct process *prc)
{
    if (!lastprocess)
    {
        prc = receiveMessage_NOWAIT(msgq_id);
        if (prc)
        {
            printf("process received and added to the queue with id %d \n", prc->id);
            lastprocess = prc->last_process;
            add(p_queue, prc);
        }
    }
    display(p_queue);
    printf("clock=%d\n", getClk());
}

void RR(char *algo, char *Qua)
{
    p_queue = (struct list *)malloc(sizeof(struct list));
    initList(p_queue);

    key_t key1 = ftok("keyfile", Mkey);
    msgq_id = init_msgq(key1);

    int Quantum=atoi(Qua);
    

    wait_time_sum = 0;
    process_num = 0;
    WTA_sum = 0;
    total_running = 0;
    lastprocess = 0;

    struct process *ptr;
    int onesecond = -1;
    shmRM = (int *)shmAttach(shmid);
    *shmRM = -1;

    // ptr->remaining_time = 0;
    int cnt = 0;

    while (!((*shmRM == -1) && isEmpty(p_queue) && lastprocess))
    {
        cnt--;
        readqueue(ptr);
        if (*shmRM == 0)
        {
            finishProcess(ptr);
            cnt = Quantum;
        }
        if (!isEmpty(p_queue) && (*shmRM <= 0 || cnt == 0))
        {
            cnt = Quantum;

            if (*shmRM != -1)
            {
                // waiting second we use
                ptr->remaining_time = *shmRM;
                add(p_queue, ptr);
                stopProcess(ptr);
            }

            struct process *first = get_first(p_queue);
            *shmRM = first->remaining_time + 1;

            ptr = first;

            if (first->remaining_time < first->runtime)
                continueProcess(ptr);
            else
                ptr->pid = startProcess(ptr);
        }
        WaitForSecond(&onesecond);
    }
    calculate();
}

void SRTN(char *algo)
{
    p_queue = (struct list *)malloc(sizeof(struct list));
    initList(p_queue);

    key_t key1 = ftok("keyfile", Mkey);
    msgq_id = init_msgq(key1);

    wait_time_sum = 0;
    process_num = 0;
    WTA_sum = 0;
    total_running = 0;
    lastprocess = 0;

    struct process *ptr;
    int onesecond = -1;
    shmRM = (int *)shmAttach(shmid);
    *shmRM = -1;

    while (!((*shmRM == -1) && isEmpty(p_queue) && lastprocess))
    {
        readqueue(ptr);
        if (*shmRM == 0)
        {
            finishProcess(ptr);
        }

        if (!isEmpty(p_queue))
        {

            struct process *first = get_first(p_queue);
            if (first->remaining_time < *shmRM)
            {
                ptr->remaining_time = *shmRM;
                add_sjf(p_queue, ptr);
                stopProcess(ptr);
            }

            if (*shmRM == -1)
            {
                *shmRM = first->remaining_time + 1;
                ptr = first;

                if (first->remaining_time < first->runtime)
                    continueProcess(ptr);
                else
                    ptr->pid = startProcess(ptr);
            }
            else
            {
                add_sjf(p_queue, first);
            }
        }
        WaitForSecond(&onesecond);
    }
    calculate();
}

void HPF(char *algo)
{

    p_queue = (struct list *)malloc(sizeof(struct list));
    initList(p_queue);

    key_t key1 = ftok("keyfile", Mkey);
    msgq_id = init_msgq(key1);

    wait_time_sum = 0;
    process_num = 0;
    WTA_sum = 0;
    total_running = 0;
    lastprocess = 0;

    struct process *ptr;
    int onesecond = -1;
    shmRM = (int *)shmAttach(shmid);
    *shmRM = -1;

    while (!((*shmRM == -1) && isEmpty(p_queue) && lastprocess))
    {
        readqueue(ptr);
        if (*shmRM == 0)
        {
            finishProcess(ptr);
        }

        if (!isEmpty(p_queue))
        {

            struct process *first = get_first(p_queue);
            if (ptr != NULL)
            {
                if (ptr->priority > first->priority)
                {
                    ptr->remaining_time = *shmRM;
                    add_PIR(p_queue, ptr);
                    stopProcess(ptr);
                }
            }

            if (*shmRM == -1)
            {
                *shmRM = first->remaining_time + 1;
                ptr = first;

                if (first->remaining_time < first->runtime)
                    continueProcess(ptr);
                else
                    ptr->pid = startProcess(ptr);
            }
            else
            {
                add_PIR(p_queue, first);
            }
        }
        WaitForSecond(&onesecond);
    }
    calculate();
}