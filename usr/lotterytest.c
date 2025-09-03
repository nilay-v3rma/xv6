#include "types.h"
#include "stat.h"
#include "user.h"

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

// Simple CPU bound test
void
simple_lottery_test(void)
{
    int pid, i;
    
    printf(1, "\n=== Simple Lottery Test ===\n");
    printf(1, "Creating 3 processes with tickets: 15, 80, 5 (3:16:1 ratio)\n");
    
    for(i = 0; i < 3; i++) {
        pid = fork();
        if(pid == 0) {
            // Child process
            int my_pid = getpid();
            int tickets[] = {15, 80, 5};
            settickets(my_pid, tickets[i]);
            
            printf(1, "Process %d starting with %d tickets\n", my_pid, tickets[i]);
            
            // CPU intensive work without much sleeping
            int work = 0;
            for(int j = 0; j < 5000; j++) {
                work += j % 7;  // Some computation
            }
            
            printf(1, "%d Process %d (tickets: %d) finished work: %d\n", i, my_pid, tickets[i], work);
            exit();
        }
    }
    
    // Wait for all children
    for(i = 0; i < 3; i++) {
        wait();
    }
    printf(1, "Simple lottery test completed\n");
}

// CPU bound timing test to show proportional scheduling
void
timing_test(void)
{
    int pid, i;
    
    printf(1, "\n=== Timing Test for Proportional Scheduling ===\n");
    printf(1, "Creating 3 processes with tickets: 30, 60, 10 (3:6:1 ratio)\n");
    printf(1, "All processes start simultaneously and compete for CPU\n");
    
    for(i = 0; i < 3; i++) {
        pid = fork();
        if(pid == 0) {
            // Child process
            int my_pid = getpid();
            int tickets[] = {30, 60, 10};
            settickets(my_pid, tickets[i]);
            
            uint start_time = uptime();
            printf(1, "Process %d starting with %d tickets at time %d\n", 
                   my_pid, tickets[i], start_time);
            
            // CPU intensive work - same amount for all processes
            // The one with more tickets should finish faster
            int work = 0;
            int target_iterations = 2000;  // Fixed amount of work
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
        }
    }
    
    // Wait for all children
    for(i = 0; i < 3; i++) {
        wait();
    }
    printf(1, "Timing test completed\n");
}

int
main(int argc, char *argv[])
{
    printf(1, "=== XV6 Lottery Scheduler Test Suite ===\n");
    
    lotterytest();
    simple_lottery_test();
    timing_test();
    
    printf(1, "\n=== All tests completed ===\n");
    exit();
}
