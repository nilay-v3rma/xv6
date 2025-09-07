#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
    int inuse[NPROC];       // whether this slot of the process table is in use (1 or 0)
    int pid[NPROC];         // PID of each process
    int base_tickets[NPROC];     // number of tickets of each process
    int boostsleft[NPROC];  // number of boosts left for each process
};

#endif // _PSTAT_H_
