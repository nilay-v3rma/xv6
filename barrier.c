#include "types.h"
#include "barrier.h"
#include "defs.h"
#include "spinlock.h"

static int barrier_n = -1;      // Total number of processes expected at barrier
static int barrier_count = 0;   // Current number of processes at barrier
static struct spinlock barrier_lock;
static int barrier_initialized = 0;

// Initialize the barrier spinlock (called during kernel initialization)
void barrier_init_lock(void) {
    initlock(&barrier_lock, "barrier");
}

// Initialize the barrier with count N
int barrier_init(uint N){
    if(N <= 0) {
        return -1; // Invalid count
    }
    
    acquire(&barrier_lock);
    
    if(barrier_initialized) {
        release(&barrier_lock);
        return -1; // Barrier already initialized
    }
    
    barrier_n = N;
    barrier_count = 0;
    barrier_initialized = 1;
    
    release(&barrier_lock);
    return 0; // Success
}

// Check if all N threads have reached the barrier
int barrier_check(void){
    acquire(&barrier_lock);
    
    // Check if barrier is initialized
    if(!barrier_initialized || barrier_n <= 0) {
        release(&barrier_lock);
        return -1; // Barrier not initialized
    }
    
    // Increment count of processes at barrier
    barrier_count++;
    
    // Check if all processes have reached the barrier
    if(barrier_count == barrier_n) {
        // Last process to arrive - wake up all waiting processes
        barrier_count = 0;
        barrier_initialized = 0; // Reset for potential reuse
        barrier_n = -1;
        wakeup(&barrier_count); // Wake up all sleeping processes
        release(&barrier_lock);
        return 0; // Barrier crossed successfully
    } else {
        // Not all processes have arrived - sleep and wait
        sleep(&barrier_count, &barrier_lock);
        release(&barrier_lock);
        return 0; // Barrier crossed after waking up
    }
}
