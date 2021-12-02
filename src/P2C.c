#include "../include/sem.h"
#include <stdio.h>
#include <sys/mman.h>
#define SHELL_NUMBER 3

// one indexed for children 0 is parent 
int my_procnum = 0; 

int factorial(int val){
    int i; 
    for(i = val-1; i>1; i--)
        val *= i; 
    return val; 
}

void task(struct sem* source, struct sem* dest){
    if(sem_try(source) == 0){
        sem_wait(source,my_procnum-1);
        sem_inc(dest);
        return;
    }
    sem_inc(dest);
}

int main(){
    struct sem* shells = (struct sem*)mmap(NULL, sizeof(struct sem)*SHELL_NUMBER, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int i; 
    for(i = 0; i < SHELL_NUMBER; i++){ // initialize semaphores
        sem_init(shells+i, 1);
    }
    int fork_num = factorial(SHELL_NUMBER);
    pid_t my_pid = getpid(), ftest, children_pid[fork_num];
    for(i = 0; i<fork_num; i++){
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
        switch(my_procnum){
            case 1:
                task(shells, shells+1);
                break;
            case 2:
                task(shells, shells+2);
                break;
            case 3:
                task(shells+1, shells);
                break;
            case 4:
                task(shells+1, shells+2);
                break;
            case 5:
                task(shells+2, shells);
                break;
            case 6:
                task(shells+2, shells+1);
                break;
            default:
                break;
        }
        int signal_handler_invoked = 0; 
        for(i = 0; i<SHELL_NUMBER; i++){
            signal_handler_invoked+=shells[i].woken_counter[my_procnum-1]; 
        }
        fprintf(stderr, "Child %d (pid %d) done, signal Handler was invoked %d times\n", my_procnum,my_pid,signal_handler_invoked); 
        return 0; 
    }

    for(i = 0; i<fork_num; i++){ // wait for all children to complete 
        waitpid(children_pid[i], NULL, 0);
        fprintf(stderr, "VCPU %d done\n", i+1); 
    }
    

    return 0; 
}