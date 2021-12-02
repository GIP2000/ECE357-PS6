#include "../include/sem.h"
#include "../include/spinlock.h"
#include <signal.h> 
#include <stdio.h>

void sem_init(struct sem* s,int count){
    s->val = count; 
    spin_init(&(s->spinlock));
    int i; 
    for(i = 0; i<N_PROC; i++)
        s->sleepers[i] = -1; 
}

int sem_try(struct sem* s){
    int return_val = 0; 
    spin_lock(&(s->spinlock));
    if(s->val > 0){
        s->val--;
        return_val = 1;        
    }
    spin_unlock(&(s->spinlock));
    fprintf(stderr, "sem_try returning %d\n",return_val);
    return return_val; 
}
void sem_wait(struct sem* s){
    spin_lock(&(s->spinlock)); 
    if(s->val > 0){
        s->val--;
        spin_unlock(&(s->spinlock));
        return; 
    } 
    // block
    int i; 
    for(i = 0; i<N_PROC; i++){
        if(s->sleepers[i] == -1){
            s->sleepers[i] = getpid(); 
            break; 
        }
    }
    sigset_t newmask,suspendmask,oldmask; 
    sigfillset(&newmask); 
    sigfillset(&suspendmask); 
    sigprocmask(SIG_BLOCK,&newmask,&oldmask); // block signals so the process can have time to sleep before been woken
    spin_unlock(&(s->spinlock)); 
    fprintf(stderr, "sem_wait sleeping\n");
    sigsuspend(&oldmask);

    // wake up
    fprintf(stderr, "sem_wait waking up\n");
    sigprocmask(SIG_SETMASK,&oldmask,NULL); // unblock signals
    s->sleepers[i] = -1;
    return sem_wait(s); // try again
    
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