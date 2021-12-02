#ifndef SPINLOCK_H
#define SPINLOCK_H

typedef char spin_t; 
// takes a pointer to a spinlock and tries to get a lock
// blocks until lock is obtained
// returns 1 when lock is obtained
// returns 0 if lock was somehow not obtained
int spin_lock(spin_t* spinlock); 


// unlocks the spinlock 
void spin_unlock(spin_t* spinlock); 


//initalizes the spinlock value to 0
void spin_init(spin_t* spinlock);


#endif