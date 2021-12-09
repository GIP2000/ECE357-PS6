#include "../include/sem.h"
#include <stdio.h>
#include <sys/mman.h>
#include <signal.h> 
#define SHELL_NUMBER 3

// one indexed for children 0 is parent 
int my_procnum = 0; 

void task(struct sem* source, struct sem* dest){
    if(sem_try(source) == 0){
        sem_wait(source,my_procnum-1);
        sem_inc(dest);
        return;
    }
    sem_inc(dest);
}

void updateTasks(int* task1, int* task2){
    *task2 = (*task2 +1)%SHELL_NUMBER;
    if(*task2 == 0)
        *task1 = (*task1 +1)%SHELL_NUMBER;
    if(*task2 == *task1)
        return updateTasks(task1,task2);
}

void childHandler(struct sem* shells,pid_t my_pid,int task1,int task2,int mult){ 
    if(task1 >= SHELL_NUMBER || task2 >= SHELL_NUMBER){
        fprintf(stderr,"Error: task1 or task2 is out of range second\n");
        exit(1); 
    }
    int i; 
    fprintf(stderr, "VCPU %d (pid %d) is doing task %d,%d\n", my_procnum, my_pid, task1, task2);
    for(i = 0; i < mult; i++)
        task(shells+task1, shells+task2);
    int signal_handler_invoked = 0; 
    int sleep_count = 0; 
    for(i = 0; i<SHELL_NUMBER; i++){
        signal_handler_invoked+=shells[i].woken_counter[my_procnum-1]; 
        sleep_count += shells[i].sleep_counter[my_procnum-1];
    }
    fprintf(stderr, "VCPU %d (pid %d) done, signal Handler was invoked %d times and I slept %d times\n", 
        my_procnum,my_pid,signal_handler_invoked,sleep_count); 

}


int main(int argc, char* argv[]){

    // initalize command line args
    int rockCount,mult;
    if (argc == 1){
        rockCount = 1; 
        mult = 1; 
    } else if (argc == 3){
        rockCount = atoi(argv[1]);
        mult = atoi(argv[2]);
        if(rockCount <= 0 || mult <= 0){
            fprintf(stderr,"Error: All Values must be positive\n Usage: ./P2C [rockCount] \n");
            exit(1); 
        }
    } else {
        fprintf(stderr,"Error: invalid number of arguments\n Usage: ./P2C [rockCount] [Multiplier]\n");
        exit(1); 
    }

    // initialize semaphores
    struct sem* shells = (struct sem*)mmap(NULL, sizeof(struct sem)*SHELL_NUMBER, 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(shells == MAP_FAILED){
        perror("mmap");
        exit(1); 
    }
    int i; 
    for(i = 0; i < SHELL_NUMBER; i++){ 
        sem_init(shells+i, rockCount);
    }

    // setup forks
    int task_num = 6,task1=0,task2 = 0;
    pid_t my_pid = getpid(), ftest, children_pid[task_num];
    for(i = 0; i<task_num; i++){
        updateTasks(&task1,&task2);
        ftest = fork(); 
        if(ftest == -1){
            perror("fork"); 
            exit(1); 
        } else if(ftest == 0){ // child
            my_procnum = i+1; 
            my_pid = getpid();
            fprintf(stderr, "VCPU %d staring, pid %d\n", my_procnum,my_pid ); 
            break; 
        } else 
            children_pid[i] = ftest;
    }
    if(ftest == 0){ // child
        childHandler(shells,my_pid,task1,task2,rockCount*mult);
        return 0; 
    }
    // parent

    for(i = 0; i<task_num; i++){ // wait for all children to complete 
        waitpid(children_pid[i], NULL, 0);
        fprintf(stderr, "VCPU %d done\n", i+1); 
    }

    for(i = 0; i<SHELL_NUMBER; i++){ // check if the answer is correct
        if(shells[i].val != rockCount){
            fprintf(stderr, "shell %d failed, val %d\n", i, shells[i].val);
            return 1; 
        }
    }
    printf("All shells passed\n");
    return 0; 
}