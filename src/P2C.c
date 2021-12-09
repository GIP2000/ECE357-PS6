#include "../include/sem.h"
#include <stdio.h>
#include <sys/mman.h>
#include <signal.h> 
#define SHELL_NUMBER 3
#define FORKS (SHELL_NUMBER*(SHELL_NUMBER-1))

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
    int i; 
    for(i = 0; i < mult; i++)
        task(shells+task1, shells+task2);
    int signal_handler_invoked = 0; 
    for(i = 0; i<SHELL_NUMBER; i++)
        signal_handler_invoked+=shells[i].woken_counter[my_procnum-1]; 
    fprintf(stderr, "Child %d (pid %d) done, signal Handler was invoked %d times\n", 
        my_procnum,my_pid,signal_handler_invoked); 

}

void shellDump(struct sem* shells){
    int i; 
    fprintf(stderr, "Sem#       val       Sleeps       Wakes\n");
    for(i = 0; i<SHELL_NUMBER; i++){ // DUMP
        fprintf(stderr, "%d           %d\n",i,shells[i].val);
        int j;
        for(j = 0; j<FORKS; j++)
            fprintf(stderr, "VCPU %d               %d       %d\n",
            j+1,shells[i].sleep_counter[j],shells[i].woken_counter[j]);
    }
}

// creates fork and handles child code
void handleForks(struct sem* shells ,int mult ){
    int task1=0,task2 = 0,i;
    pid_t my_pid = getpid(), ftest, children_pid[FORKS];
    for(i = 0; i<FORKS; i++){
        updateTasks(&task1,&task2);
        ftest = fork(); 
        if(ftest == -1){
            perror("fork"); 
            exit(1); 
        } else if(ftest == 0){ // child
            my_procnum = i+1; 
            my_pid = getpid();
            fprintf(stderr, "VCPU %d staring, pid %d\n", my_procnum,my_pid ); 
            childHandler(shells,my_pid,task1,task2,mult);
            exit(0);  
        } else 
            children_pid[i] = ftest;
    }

    // wait for all children to finish
    fprintf(stderr, "Main process spawned all children, waiting\n"); 
    for(i = 0; i<FORKS; i++){ // wait for all children to complete 
        waitpid(children_pid[i], NULL, 0);
        fprintf(stderr, "VCPU %d exited\n", i+1); 
    }
}

void init_args(int argc, char* argv[], int* rockCount, int* mult){
    if (argc == 1){
        *rockCount = 1; 
        *mult = 1; 
    } else if (argc == 3){
        *rockCount = atoi(argv[1]);
        *mult = atoi(argv[2]);
        if((*rockCount) <= 0 || (*mult) <= 0){
            fprintf(stderr,"Error: All Values must be positive\n Usage: ./P2C [rockCount] [Multiplier]\n");
            exit(1); 
        }
    } else {
        fprintf(stderr,"Error: invalid number of arguments\n Usage: ./P2C [rockCount] [Multiplier]\n");
        exit(1); 
    }
}

void init_shells(struct sem** shells, int rockCount){
    *shells = (struct sem*)mmap(NULL, sizeof(struct sem)*SHELL_NUMBER, 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if((*shells) == MAP_FAILED){
        perror("mmap");
        exit(1); 
    }
    int i; 
    for(i = 0; i < SHELL_NUMBER; i++){
        sem_init((*shells)+i, rockCount);
    }
}

int assertCorrectAnswer(struct sem* shells, int rockCount){
    int i,ret = 0;
    for(i = 0; i<SHELL_NUMBER; i++){ // check if the answer is correct
        if(shells[i].val != rockCount){
            printf("Shell %d failed, val %d\n", i, shells[i].val);
            ret = 1;
        }
    }
    shellDump(shells); 
    if(ret == 0) printf("All shells passed\n");
    return ret;
}


int main(int argc, char* argv[]){

    // initalize command line args
    int rockCount,mult;
    init_args(argc,argv,&rockCount,&mult);

    // initialize semaphores
    struct sem* shells;
    init_shells(&shells,rockCount);
    
    // create forks
    handleForks(shells,rockCount*mult);

    // Waiting is completed in handle forks.
    // Parent should be running with no children running. 

    return assertCorrectAnswer(shells,rockCount);

}