#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "lottery_assets.h"

// External declaration for ptable
extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;

struct proc *hold_lottery(int total_tickets, struct proc *proc_table){
    int winning_ticket = srand() % total_tickets;
    int current_count = 0;
    struct proc *p;

    for(p = proc_table; p < &proc_table[NPROC]; p++) {
        if(p->state == RUNNABLE) {
            current_count += p->tickets;
            if(current_count > winning_ticket) {
                return p;
            }
        }
    }
    return 0;
}

void update_boosted_tickets(struct proc *proc_table){
    struct proc *p;

    for(p = proc_table; p < &proc_table[NPROC]; p++) {
        if(p->state == RUNNABLE) {
            if(p->boost_ticks > 0) {
                p->boost_ticks--;
                if(p->boost_ticks == 0) {
                    p->tickets = p->base_tickets;
                }
                else{
                    p->tickets = p->base_tickets * 2;
                }
            }
        }
        if(p->state == SLEEPING) {
            p->boost_ticks++;
        }

    }
}

// Wrapper function that doesn't require ptable parameter
void update_boosted_tickets_timer(void) {
    update_boosted_tickets(ptable.proc);
}

// Update sleeping processes - wake them up if their sleep duration has expired
void update_sleeping_processes(struct proc *proc_table) {
    struct proc *p;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if(p->state == SLEEPING && p->sleep_duration > 0) {
            if(ticks - p->sleep_start >= p->sleep_duration) {
                p->state = RUNNABLE;
                p->sleep_duration = 0;
                // When a process wakes up from sleep, boost its tickets
                if(p->boost_ticks == 0) {
                    uint sleep_time = ticks - p->sleep_start;
                    p->boost_ticks = sleep_time;
                    p->tickets = p->base_tickets * 2;  // Double the tickets
                }
            }
        }
    }
}

// Wrapper function that doesn't require ptable parameter
void update_sleeping_processes_wrapper(void) {
    update_sleeping_processes(ptable.proc);
}

int settickets(int pid, int tickets) {
    struct proc *p;
    
    cprintf("settickets called: pid=%d, tickets=%d\n", pid, tickets);  // Debug output
    
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if(p->pid == pid) {
            p->base_tickets = tickets;
            if(p->boost_ticks == 0) {
                p->tickets = tickets;
            } else {
                p->tickets = p->base_tickets * 2;
            }
            cprintf("Process %d tickets updated to %d\n", pid, tickets);  // Debug output
            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);
    cprintf("Process %d not found\n", pid);  // Debug output
    return -1;
}

