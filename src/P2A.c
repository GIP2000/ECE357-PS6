#include "../include/spinlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdbool.h>
#define N_PROC 64

struct numberToAdd{
    unsigned int number; 
    spin_t lock; 
};

int my_procnum = 0; 
pid_t pid; 

void addWithSpinLock(struct numberToAdd *n){
    spin_lock(&n->lock);
    n->number = n->number + 1; 
    spin_unlock(&n->lock);
}


int main(int argc, char *argv[]){
    bool useSpinLock = true;
    if(argc == 2){
        useSpinLock = strcmp(argv[1],"true") == 0; 
        if(!useSpinLock)
            useSpinLock = !strcmp(argv[1],"false") == 0; 
        else{
            fprintf(stderr, "Invalid argument. Please insert true or false \n");
            exit(1); 
        }
    } else if(argc > 2){
        fprintf(stderr, "Invalid argument. Please insert true or false \n");
        exit(1); 
    }
    printf("%s\n", useSpinLock ? "Using spinlock" : "Not using spinlock");

    pid = getpid(); 
    pid_t children_pids[N_PROC]; 
    struct numberToAdd *number = (struct numberToAdd *)mmap(NULL, sizeof(struct numberToAdd), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    spin_init(&number->lock);
    number->number = 0;

    int i; 
    int ftest; 
    for(i = 0; i<N_PROC; i++){
        ftest = fork(); 
        if(ftest == -1){
            fprintf(stderr, "fork error on %d: %s\n", i, strerror(errno));
            exit(1);
        } else if(ftest == 0){
            my_procnum = i+1; 
            pid = getpid();
            break;
        } 
        children_pids[i] = ftest;
    }
    for (i = 0; i<10000; i++){
        if(useSpinLock)
            addWithSpinLock(number);
        else
            number->number = number->number + 1; 
    }
    if(ftest != 0){
        for(i = 0; i<N_PROC; i++){
            waitpid(children_pids[i], NULL, 0);
        }
        printf("Final Value: %d\n", number->number);
    }
    return 0; 
}
