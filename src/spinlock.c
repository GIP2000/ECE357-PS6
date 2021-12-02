#include "../include/spinlock.h"
#include "../include/tas.h"


int spin_lock(spin_t* spinlock){
    while(tas(spinlock) != 0)
        ; // spin
    return *spinlock; 
} 

void spin_unlock(spin_t* spinlock){
    *spinlock = 0;
} 

void spin_init(spin_t* spinlock){
    *spinlock = 0;
}