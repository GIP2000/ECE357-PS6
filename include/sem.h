#ifndef SEM_H
#define SEM_H
#include "spinlock.h"
#include <stdlib.h>
#define N_PROC 64

struct sem{
    int val; 
    spin_t spinlock; 
    pid_t sleepers[N_PROC]; // location to keep values
};

int sem_try(struct sem* s); 
void sem_wait(struct sem* s);
void sem_init(struct sem* s,int count); 
void sem_inc(struct sem* s);



#endif