#include "../include/sem.h"
#include <stdio.h>
#include <sys/mman.h>
#define SHELL_NUMBER 3

int my_procnum = 0; 

int factorial(int val){
    int i; 
    for(i = val-1; i>1; i--)
        val *= i; 
    return val; 
}

void task(struct sem* source, struct sem* dest){
    if(sem_try(source) == 0){
        fprintf(stderr, "before wait\n");
        sem_wait(source);
        fprintf(stderr, "after wait\n");
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
    fprintf(stderr, "fork_num: %d\n", fork_num);
    pid_t my_pid = getpid(), ftest, children_pid[fork_num];
    for(i = 0; i<fork_num; i++){
        ftest = fork(); 
        if(ftest == -1){
            perror("fork"); 
            exit(1); 
        } else if(ftest == 0){ // child
            my_procnum = i+1; 
            my_pid = getpid();
            break; 
        } else 
            children_pid[i] = ftest;
    }
    if(ftest == 0){ // child
        fprintf(stderr, "I am child %d of %d\n", my_procnum, fork_num);
        switch(my_procnum){
            case 1:
                task(shells+0, shells+1);
                break;
            case 2:
                task(shells+0, shells+2);
                break;
            case 3:
                task(shells+1, shells+0);
                break;
            case 4:
                task(shells+1, shells+2);
                break;
            case 5:
                task(shells+2, shells+0);
                break;
            case 6:
                task(shells+2, shells+1);
                break;
            default:
                break;
        }
        // int val = sem_try(shells+my_procnum-1); 
        // fprintf(stderr, "try_sem -> %s\n",  val == 1 ? "true" : "false");
        return 0; 
    }
    for(i = 0; i<fork_num; i++){
        waitpid(children_pid[i], NULL, 0);
    }

    return 0; 
}