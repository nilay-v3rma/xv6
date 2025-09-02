#ifndef LOTTERY_ASSETS_H
#define LOTTERY_ASSETS_H

#include "proc.h"

// Function declarations for lottery scheduling
struct proc *hold_lottery(int total_tickets, struct proc *proc_table);
void update_boosted_tickets(struct proc *proc_table);
void update_boosted_tickets_timer(void);
void update_boosted_tickets(struct proc *proc_table);
void update_sleeping_processes_wrapper(void);
void update_sleeping_processes(struct proc *proc_table);
int settickets(int pid, int tickets);

#endif // LOTTERY_ASSETS_H
