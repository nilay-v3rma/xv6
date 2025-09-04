#include "types.h"
#include "stat.h"
#include "user.h"

// Function prototypes
void lotterytest(void);
void timing_test(void);
void boost_test(void);
void boost_decay_test(void);

// Test lottery scheduler by creating processes with different ticket counts
void
lotterytest(void)
{
    int pid, i, j;
    int total_iterations = 500;  // Increased iterations
    
    printf(1, "Lottery Scheduler Test Starting...\n");
    
    // Test 1: Basic settickets functionality
    printf(1, "\nTest 1: Basic settickets functionality\n");
    pid = getpid();
    printf(1, "Current process PID: %d\n", pid);
    
    if(settickets(pid, 10) == 0) {
        printf(1, "SUCCESS: settickets(%d, 10) succeeded\n", pid);
    } else {
        printf(1, "FAIL: settickets(%d, 10) failed\n", pid);
        return;
    }
    
    // Test 2: Invalid parameters
    printf(1, "\nTest 2: Testing invalid parameters\n");
    if(settickets(pid, 0) == -1) {
        printf(1, "SUCCESS: settickets with 0 tickets properly rejected\n");
    } else {
        printf(1, "FAIL: settickets with 0 tickets should be rejected\n");
    }
    
    if(settickets(pid, -5) == -1) {
        printf(1, "SUCCESS: settickets with negative tickets properly rejected\n");
    } else {
        printf(1, "FAIL: settickets with negative tickets should be rejected\n");
    }
    
    // Test 3: Fork children with different ticket counts
    printf(1, "\nTest 3: Creating processes with different ticket priorities\n");
    
    for(i = 0; i < 2; i++) {
        pid = fork();
        if(pid < 0) {
            printf(1, "FAIL: fork failed\n");
            return;
        }
        
        if(pid == 0) {
            // Child process
            int my_pid = getpid();
            int my_tickets = (i == 0) ? 5 : 50;  // 1:10 ratio for clearer demonstration
            
            if(settickets(my_pid, my_tickets) == 0) {
                printf(1, "Child %d (PID: %d) set tickets to %d\n", i, my_pid, my_tickets);
            } else {
                printf(1, "Child %d (PID: %d) failed to set tickets\n", i, my_pid);
                exit();
            }
            
            // Do some CPU-intensive work to test scheduling
            int counter = 0;
            uint start_time = uptime();
            for(j = 0; j < total_iterations; j++) {
                counter++;
                // Reduce sleeping to better test CPU scheduling
                if(j % 500 == 0) {
                    sleep(1);
                }
            }
            uint end_time = uptime();
            
            printf(1, "Child %d (tickets: %d) completed %d iterations in %d ticks\n", 
                   i, my_tickets, counter, end_time - start_time);
            exit();
        }
    }
    
    // Parent waits for children and counts results
    printf(1, "Parent waiting for children to complete...\n");
    for(i = 0; i < 2; i++) {
        wait();
    }
    
    printf(1, "\nTest 4: Testing settickets on non-existent process\n");
    if(settickets(99999, 10) == -1) {
        printf(1, "SUCCESS: settickets on non-existent PID properly rejected\n");
    } else {
        printf(1, "FAIL: settickets on non-existent PID should be rejected\n");
    }
    
    printf(1, "\nLottery Scheduler Test Completed!\n");
}

// CPU bound timing test to show proportional scheduling
void
timing_test(void)
{
    int pids[3];
    int i;
    
    printf(1, "\n=== Timing Test for Proportional Scheduling ===\n");
    printf(1, "Creating 3 processes with tickets: 30, 60, 10 (3:6:1 ratio)\n");
    printf(1, "All processes start simultaneously and compete for CPU\n");
    
    // Phase 1: Fork all processes first
    for(i = 0; i < 3; i++) {
        pids[i] = fork();
        if(pids[i] == 0) {
            // Child process - set up tickets but don't start work yet
            int my_pid = getpid();
            int tickets[] = {30, 60, 10};
            settickets(my_pid, tickets[i]);
            
            printf(1, "Process %d created with %d tickets, waiting for start signal...\n", 
                   my_pid, tickets[i]);
            
            // Wait a bit to ensure all processes are created and ready
            sleep(10);  // Give time for all forks to complete
            
            // Now all processes start their work simultaneously
            uint start_time = uptime();
            printf(1, "Process %d starting work at time %d\n", my_pid, start_time);
            
            // CPU intensive work - same amount for all processes
            // The one with more tickets should get more CPU time
            int work = 0;
            int target_iterations = 3000;  // Fixed amount of work
            for(int j = 0; j < target_iterations; j++) {
                work += j % 13;  // Some computation
                // Occasionally yield to allow scheduler to run
                if(j % 1000 == 0) {
                    sleep(1);  // Brief yield to scheduler
                }
            }
            
            uint end_time = uptime();
            printf(1, "Process %d (tickets: %d) finished %d iterations at time %d (duration: %d ticks)\n", 
                   my_pid, tickets[i], target_iterations, end_time, end_time - start_time);
            exit();
        } else if(pids[i] < 0) {
            printf(1, "FAIL: fork failed for process %d\n", i);
            return;
        }
    }
    
    printf(1, "All 3 processes forked successfully, they will start work after synchronization\n");
    
    // Wait for all children
    for(i = 0; i < 3; i++) {
        wait();
    }
    printf(1, "Timing test completed\n");
}

// Test ticket boosting mechanism
void
boost_test(void)
{
    int pid, i;
    
    printf(1, "\n=== Ticket Boosting Test ===\n");
    printf(1, "Testing priority boost for I/O bound processes\n");
    printf(1, "Creating 3 processes: 2 CPU-bound (10 tickets), 1 I/O-bound (10 tickets)\n");
    printf(1, "The I/O-bound process should get boosted tickets and finish faster due to better scheduling\n");
    
    for(i = 0; i < 3; i++) {
        pid = fork();
        if(pid == 0) {
            // Child process
            int my_pid = getpid();
            settickets(my_pid, 10);  // All start with same tickets
            
            if(i < 2) {
                // CPU-bound processes (no boosting expected)
                printf(1, "CPU-bound Process %d (PID: %d) starting\n", i + 1, my_pid);
                
                uint start_time = uptime();
                int work = 0;
                // Pure CPU work - no sleeping, so no boost
                for(int j = 0; j < 2500; j++) {
                    work += j % 17;
                    // No sleep calls - stays CPU bound
                }
                uint end_time = uptime();
                
                printf(1, "CPU-bound Process %d finished at time %d (duration: %d ticks)\n", 
                       i + 1, end_time, end_time - start_time);
                
            } else {
                // I/O-bound process (should get boosting)
                printf(1, "I/O-bound Process (PID: %d) starting - will sleep frequently\n", my_pid);
                
                uint start_time = uptime();
                int work = 0;
                
                // I/O simulation: do work then sleep repeatedly
                for(int j = 0; j < 10; j++) {
                    // Do some work
                    for(int k = 0; k < 250; k++) {
                        work += k % 13;
                    }
                    
                    // Simulate I/O by sleeping (this should accumulate boost_ticks)
                    printf(1, "I/O Process sleeping (iteration %d) - should gain boost\n", j + 1);
                    sleep(5);  // Sleep for 5 ticks - this should build up boost
                    printf(1, "I/O Process woke up (iteration %d) - should now have boosted tickets\n", j + 1);
                }
                
                uint end_time = uptime();
                printf(1, "I/O-bound Process finished at time %d (duration: %d ticks)\n", 
                       end_time, end_time - start_time);
            }
            exit();
        } else if(pid < 0) {
            printf(1, "FAIL: fork failed for process %d\n", i);
            return;
        }
    }
    
    printf(1, "All processes started. The I/O-bound process should benefit from ticket boosting.\n");
    
    // Wait for all children
    for(i = 0; i < 3; i++) {
        wait();
    }
    
    printf(1, "Boost test completed\n");
    printf(1, "Expected behavior: I/O-bound process should get more CPU time due to boosting\n");
    printf(1, "and potentially finish sooner despite doing the same amount of work.\n");
}

// Test boost decay mechanism  
void
boost_decay_test(void)
{
    int pid;
    
    printf(1, "\n=== Boost Decay Test ===\n");
    printf(1, "Testing that boost tickets decay over time\n");
    
    pid = fork();
    if(pid == 0) {
        int my_pid = getpid();
        settickets(my_pid, 5);  // Start with 5 base tickets
        
        printf(1, "Process %d: Starting with 5 base tickets\n", my_pid);
        
        // First, accumulate some boost by sleeping
        printf(1, "Process %d: Sleeping to accumulate boost...\n", my_pid);
        sleep(10);  // Sleep for 10 ticks to build boost
        printf(1, "Process %d: Woke up - should now have boosted tickets (10 = 5*2)\n", my_pid);
        
        // Now do CPU work while boost decays
        printf(1, "Process %d: Starting CPU work - boost should decay over time\n", my_pid);
        uint start_time = uptime();
        
        for(int i = 0; i < 20; i++) {
            // Do some work each iteration
            for(int j = 0; j < 100; j++) {
                int dummy = j % 7;
                (void)dummy;  // Avoid unused variable warning
            }
            
            // Check time periodically
            if(i % 5 == 0) {
                uint current_time = uptime();
                printf(1, "Process %d: Iteration %d, time elapsed: %d ticks\n", 
                       my_pid, i, current_time - start_time);
            }
            
            // Brief yield to let scheduler run
            sleep(1);
        }
        
        uint end_time = uptime();
        printf(1, "Process %d: Finished CPU work at time %d (duration: %d ticks)\n", 
               my_pid, end_time, end_time - start_time);
        printf(1, "Process %d: Boost should have decayed back to base tickets (5)\n", my_pid);
        
        exit();
    } else if(pid < 0) {
        printf(1, "FAIL: fork failed for boost decay test\n");
        return;
    }
    
    wait();
    printf(1, "Boost decay test completed\n");
}

int
main(int argc, char *argv[])
{
    printf(1, "=== XV6 Lottery Scheduler Test Suite ===\n");
    
    lotterytest();
    timing_test();
    boost_test();
    boost_decay_test();
    
    printf(1, "\n=== All tests completed ===\n");
    exit();
}
