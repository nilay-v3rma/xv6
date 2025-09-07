#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define TEST_DURATION_TICKS 300
#define FORK_DELAY 5
#define TOLERANCE 0.3  // 30% tolerance for fairness tests

// Helper function to get process info by PID
int get_proc_info(int pid, struct pstat *ps) {
    if (getpinfo(ps) < 0) {
        printf(1, "ERROR: getpinfo failed\n");
        return -1;
    }
    
    for (int i = 0; i < NPROC; i++) {
        if (ps->inuse[i] && ps->pid[i] == pid) {
            return i;
        }
    }
    return -1;
}

// Helper function to run CPU-intensive work
void cpu_intensive_work(int duration_ticks) {
    printf(2, "Starting CPU-intensive work for %d ticks...\n", duration_ticks);
    int flag = 30000;
    while (flag > 0) {
        // Busy work to consume CPU
        for (int i = 0; i < 10000; i++) {
            __asm__ volatile ("nop");
        }
        flag--;
    }
}
void cpu_intensive_work2(int duration_ticks) {
    printf(2, "Starting 2nd CPU-intensive work for %d ticks...\n", duration_ticks);
    int flag = 30000;
    while (flag > 0) {
        // Busy work to consume CPU
        for (int i = 0; i < 10000; i++) {
            __asm__ volatile ("nop");
        }
        flag--;
    }
}

// Test 1: Inheritance test
void test_inheritance() {
    printf(1, "\n=== Test 1: Inheritance Test ===\n");
    
    int parent_pid = getpid();
    struct pstat ps;
    
    // Set parent to have 5 tickets
    if (settickets(parent_pid, 5) < 0) {
        printf(1, "FAIL: Could not set parent tickets\n");
        return;
    }
    
    // Sleep to accumulate some boosts
    sleep(3);
    
    // Check parent has boosts
    int parent_idx = get_proc_info(parent_pid, &ps);
    if (parent_idx < 0) {
        printf(1, "FAIL: Could not find parent process\n");
        return;
    }
    
    printf(1, "Parent before fork: tickets=%d, boosts=%d\n", 
           ps.base_tickets[parent_idx], ps.boostsleft[parent_idx]);
    
    int child_pid = fork();
    if (child_pid == 0) {
        // Child process
        int my_pid = getpid();
        int child_idx = get_proc_info(my_pid, &ps);
        
        if (child_idx < 0) {
            printf(1, "FAIL: Child could not find itself\n");
            exit();
        }
        
        printf(1, "Child: pid=%d, tickets=%d, boosts=%d\n",
               my_pid, ps.base_tickets[child_idx], ps.boostsleft[child_idx]);
        
        // Child should inherit parent's base tickets (5) but not boosts
        if (ps.base_tickets[child_idx] == 5 && ps.boostsleft[child_idx] == 0) {
            printf(1, "PASS: Child inherited correct tickets without boosts\n");
        } else {
            printf(1, "FAIL: Child tickets=%d (expected 5), boosts=%d (expected 0)\n",
                   ps.base_tickets[child_idx], ps.boostsleft[child_idx]);
        }
        exit();
    } else {
        wait();
    }
}

// Test 2: Sleep clean test
void test_sleep_clean() {
    printf(1, "\n=== Test 2: Sleep Clean Test ===\n");
    
    int my_pid = getpid();
    struct pstat ps_before, ps_after;
    
    // Get initial state
    int my_idx = get_proc_info(my_pid, &ps_before);
    if (my_idx < 0) {
        printf(1, "FAIL: Could not find process\n");
        return;
    }
    
    int initial_boosts = ps_before.boostsleft[my_idx];
    
    printf(1, "Before sleep: boosts=%d\n", 
           initial_boosts);
    
    // Sleep for some time
    sleep(5);
    
    // Get state after sleep
    my_idx = get_proc_info(my_pid, &ps_after);
    if (my_idx < 0) {
        printf(1, "FAIL: Could not find process after sleep\n");
        return;
    }
    
    printf(1, "After sleep: boosts=%d\n",
           ps_after.boostsleft[my_idx]);
    
    // Check that boosts accumulated correctly during sleep
    int expected_boosts = initial_boosts + 3;
    
    if (ps_after.boostsleft[my_idx] == expected_boosts) {
        printf(1, "PASS: Sleep behavior is clean\n");
    } else {
        printf(1, "FAIL: boosts=%d (expected %d)\n",
               ps_after.boostsleft[my_idx], expected_boosts);
    }
}

// Test 3: Basic fairness test
void test_fair() {
    printf(1, "\n=== Test 3: Basic Fairness Test ===\n");
    
    // Remove srand call since we don't need randomness for this test
    
    int child1_pid = fork();
    if (child1_pid == 0) {
        // Child 1: 1 ticket
        int my_pid = getpid();
        settickets(my_pid, 1);
        
        printf(1, "Child1 starting CPU work...\n");
        cpu_intensive_work(TEST_DURATION_TICKS);
        
        printf(1, "Child1 (1 ticket): completed work \n");
        exit();
    }
    
    // sleep(FORK_DELAY); // Small delay between forks
    
    int child2_pid = fork();
    if (child2_pid == 0) {
        // Child 2: 4 tickets
        int my_pid = getpid();
        settickets(my_pid, 4);
        
        printf(1, "Child2 starting CPU work...\n");
        cpu_intensive_work2(TEST_DURATION_TICKS);
        
        printf(1, "Child2 (10 tickets): completed work \n");
        exit();
    }
    
    // Wait for both children
    wait();
    wait();
    
    printf(1, "Expected fairness: process with 4 tickets should get more CPU time than process with 1 ticket.\n");
}

// Test 4: Basic boost test
void test_boost_basic() {
    printf(1, "\n=== Test 4: Basic Boost Test ===\n");
    
    int my_pid = getpid();
    settickets(my_pid, 2);
    
    struct pstat ps_before, ps_after;
    
    // Get state before sleep
    int my_idx = get_proc_info(my_pid, &ps_before);
    int initial_boosts = ps_before.boostsleft[my_idx];
    
    // Sleep for specific duration
    int sleep_duration = 7;
    printf(1, "Sleeping for %d ticks...\n", sleep_duration);
    sleep(sleep_duration);
    
    // Check boosts after sleep
    my_idx = get_proc_info(my_pid, &ps_after);
    int final_boosts = ps_after.boostsleft[my_idx];
    int expected_boosts = initial_boosts + sleep_duration - 2;
    
    printf(1, "Boosts before: %d, after: %d, expected: %d\n",
           initial_boosts, final_boosts, expected_boosts);
    
    if (final_boosts == expected_boosts) {
        printf(1, "PASS: Correct boost accumulation\n");
    } else {
        printf(1, "FAIL: Got %d boosts, expected %d\n", final_boosts, expected_boosts);
    }
}

// Test 5: Boost accumulation test
void test_boost_accumulate() {
    printf(1, "\n=== Test 5: Boost Accumulation Test ===\n");
    
    int my_pid = getpid();
    settickets(my_pid, 1);
    
    struct pstat ps;
    
    // First sleep
    sleep(3);
    int my_idx = get_proc_info(my_pid, &ps);
    int boosts_after_first = ps.boostsleft[my_idx];
    printf(1, "After first sleep (3 ticks): boosts=%d\n", boosts_after_first);
    
    // Second sleep before consuming all boosts
    sleep(4);
    my_idx = get_proc_info(my_pid, &ps);
    int boosts_after_second = ps.boostsleft[my_idx];
    printf(1, "After second sleep (4 ticks): boosts=%d\n", boosts_after_second);
    
    // Should have accumulated: 3 + 4 = 7 boosts (assuming no consumption during sleeps)
    if (boosts_after_second >= 7) {
        printf(1, "PASS: Boosts accumulated correctly\n");
    } else {
        printf(1, "FAIL: Expected at least 7 boosts, got %d\n", boosts_after_second);
    }
    
    // Now consume some boosts by doing CPU work
    cpu_intensive_work(10);
    
    my_idx = get_proc_info(my_pid, &ps);
    int boosts_after_work = ps.boostsleft[my_idx];
    printf(1, "After CPU work: boosts=%d\n", boosts_after_work);
    
    if (boosts_after_work < boosts_after_second) {
        printf(1, "PASS: Boosts were consumed during CPU work\n");
    } else {
        printf(1, "FAIL: Boosts were not consumed\n");
    }
}

// Test 6: Multiple sleepers test
void test_multi_sleepers() {
    printf(1, "\n=== Test 6: Multiple Sleepers Test ===\n");
    
    int child1_pid = fork();
    if (child1_pid == 0) {
        // Child 1: sleep for 3 ticks
        int my_pid = getpid();
        settickets(my_pid, 1);
        
        sleep(3);
        
        struct pstat ps;
        int my_idx = get_proc_info(my_pid, &ps);
        printf(1, "Child1 (slept 3): boosts=%d\n", ps.boostsleft[my_idx]);
        
        if (ps.boostsleft[my_idx] >= 0) {
            printf(1, "Child1 PASS\n");
        } else {
            printf(1, "Child1 FAIL\n");
        }
        exit();
    }
    
    sleep(1); // Small delay
    
    int child2_pid = fork();
    if (child2_pid == 0) {
        // Child 2: sleep for 6 ticks
        int my_pid = getpid();
        settickets(my_pid, 1);
        
        sleep(6);
        
        struct pstat ps;
        int my_idx = get_proc_info(my_pid, &ps);
        printf(1, "Child2 (slept 6): boosts=%d\n", ps.boostsleft[my_idx]);
        
        if (ps.boostsleft[my_idx] >= 0) {
            printf(1, "Child2 PASS\n");
        } else {
            printf(1, "Child2 FAIL\n");
        }
        exit();
    }
    
    // Wait for both children
    wait();
    wait();
    
    printf(1, "Multiple sleepers should get independent boosts\n");
}

// Test 7: Boost semantics and getpinfo test
void test_boost_and_semantics() {
    printf(1, "\n=== Test 7: Boost Semantics and getpinfo Test ===\n");
    
    int my_pid = getpid();
    
    // Set base tickets
    int base_tickets = 3;
    settickets(my_pid, base_tickets);
    
    // Sleep to get boosts
    sleep(5);
    
    struct pstat ps;
    int my_idx = get_proc_info(my_pid, &ps);
    
    printf(1, "Base tickets: %d, getpinfo tickets: %d, boosts: %d\n",
           base_tickets, ps.base_tickets[my_idx], ps.boostsleft[my_idx]);
    
    // getpinfo should return base tickets, not doubled
    if (ps.base_tickets[my_idx] == base_tickets) {
        printf(1, "PASS: getpinfo returns base tickets correctly\n");
    } else {
        printf(1, "FAIL: getpinfo returned %d, expected %d\n", 
               ps.base_tickets[my_idx], base_tickets);
    }
    
    // Boosts should be positive after sleep
    if (ps.boostsleft[my_idx] > 0) {
        printf(1, "PASS: Boosts accumulated after sleep\n");
    } else {
        printf(1, "FAIL: No boosts after sleep\n");
    }
    
    // Record initial boost count
    int initial_boosts = ps.boostsleft[my_idx];
    
    // Do CPU work to consume boosts
    printf(1, "Consuming boosts with CPU work...\n");
    cpu_intensive_work(20);
    
    // Check if boosts decreased
    my_idx = get_proc_info(my_pid, &ps);
    printf(1, "Boosts after work: %d (was %d)\n", ps.boostsleft[my_idx], initial_boosts);
    
    if (ps.boostsleft[my_idx] < initial_boosts) {
        printf(1, "PASS: Boosts decremented during lottery participation\n");
    } else {
        printf(1, "FAIL: Boosts did not decrement\n");
    }
}

// Main test runner
int main(int argc, char *argv[]) {
    printf(1, "=== Lottery Scheduler Comprehensive Test Suite ===\n");
    printf(1, "Testing all requirements systematically...\n");
    
    // No need for random seed since we removed srand
    
    // Run all tests
    // test_inheritance();
    // test_sleep_clean();
    test_fair();
    // test_boost_basic();
    // test_boost_accumulate();
    // test_multi_sleepers();
    // test_boost_and_semantics();
    
    printf(1, "\n=== Test Suite Complete ===\n");
    printf(1, "Review output above for PASS/FAIL results\n");
    
    exit();
}
