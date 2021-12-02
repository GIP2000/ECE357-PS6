#ifndef SEM_H
#define SEM_H
#include "spinlock.h"
#include <stdlib.h>
#define N_PROC 64

struct sem{
    int val; 
    spin_t spinlock; 
    pid_t sleepers[N_PROC]; // location to keep values
    int sleep_counter[N_PROC]; 
    int woken_counter[N_PROC]; 
};

// initis the semaphore to the count and initalizes all the countesr
void sem_init(struct sem* s,int count); 

// trys to decrement if it would block returns 0
//Otherwseise decrements and retruns 1
int sem_try(struct sem* s); 

// Trys to decrement if not blocks until resources are available 
void sem_wait(struct sem* s, int my_procnum);

// Increments the semaphmore if it is now positive wake all sleeping processes 
void sem_inc(struct sem* s);



#endif