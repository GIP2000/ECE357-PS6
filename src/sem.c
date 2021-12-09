#include "../include/sem.h"
#include "../include/spinlock.h"
#include <signal.h> 
#include <stdio.h>

void signalHandler(int signalNum){
    return; 
}

void sem_init(struct sem* s,int count){
    s->val = count; 
    spin_init(&(s->spinlock));
    int i; 
    for(i = 0; i<N_PROC; i++){
        s->sleepers[i] = -1; 
        s->sleep_counter[i] = 0; 
        s->woken_counter[i] = 0; 
    }
}

int sem_try(struct sem* s){
    int return_val = 0; 
    spin_lock(&(s->spinlock));
    if(s->val > 0){
        s->val--;
        return_val = 1;        
    }
    spin_unlock(&(s->spinlock));
    return return_val; 
}
void sem_wait(struct sem* s, int my_procnum ){
    spin_lock(&(s->spinlock)); 
    if(s->val > 0){
        s->val--;
        spin_unlock(&(s->spinlock));
        return; 
    } 
    // block
    sigset_t newmask,oldmask; 
    sigfillset(&newmask); 
    signal(SIGUSR1,signalHandler); 
    sigprocmask(SIG_BLOCK,&newmask,&oldmask); // block signals so the process can have time to sleep before been woken
    
    s->sleepers[my_procnum] = getpid();
    
    s->sleep_counter[my_procnum]++; 
    spin_unlock(&(s->spinlock)); 
    sigsuspend(&oldmask);

    // wake up
    s->woken_counter[my_procnum]++; 
    sigprocmask(SIG_SETMASK,&oldmask,NULL); // unblock signals
    signal(SIGUSR1,SIG_DFL); 
    s->sleepers[my_procnum] = -1;
    return sem_wait(s,my_procnum); // try again
    
}

void sem_inc(struct sem* s){
    spin_lock(&(s->spinlock)); 
    s->val++; 
    if(s->val > 0){
        int i; 
        for(i = 0; i<N_PROC; i++){
            if(s->sleepers[i] == -1) continue;  
            kill(s->sleepers[i],SIGUSR1); 
            s->sleepers[i] = -1; 
        }
    }
    spin_unlock(&(s->spinlock)); 
}