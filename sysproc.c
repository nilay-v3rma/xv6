#include "types.h"
#include "arm.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void)
{
    return fork();
}

int sys_exit(void)
{
    exit();
    return 0;  // not reached
}

int sys_wait(void)
{
    return wait();
}

int sys_kill(void)
{
    int pid;

    if(argint(0, &pid) < 0) {
        return -1;
    }

    return kill(pid);
}

int sys_getpid(void)
{
    return proc->pid;
}

int sys_sbrk(void)
{
    int addr;
    int n;

    if(argint(0, &n) < 0) {
        return -1;
    }

    addr = proc->sz;

    if(growproc(n) < 0) {
        return -1;
    }

    return addr;
}

int sys_sleep(void)
{
    int n;
    uint ticks0;

    if(argint(0, &n) < 0) {
        return -1;
    }

    acquire(&tickslock);

    ticks0 = ticks;

    while(ticks - ticks0 < n){
        if(proc->killed){
            release(&tickslock);
            return -1;
        }

        sleep(&ticks, &tickslock);
    }

    release(&tickslock);
    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void)
{
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);

    return xticks;
}

int sys_barrier_init(void)
{
    int n;
    
    if(argint(0, &n) < 0) {
        return -1;
    }
    
    return barrier_init(n);
}

int sys_barrier_check(void)
{
    return barrier_check();
}

// New code goes here
int sys_thread_create(void){
    uint* thread;
    void* (*func)(void*);
    void* arg;
    
    // Extract first argument: pointer to uint (where thread_id will be stored)
    if(argptr(0, (char**)&thread, sizeof(uint*)) < 0) {
        return -1;
    }
    
    // Extract second argument: function pointer
    if(argint(1, (int*)&func) < 0) {
        return -1;
    }
    
    // Extract third argument: void* argument to pass to the function
    if(argint(2, (int*)&arg) < 0) {
        return -1;
    }
    
    return thread_create(thread, func, arg);
}

int sys_thread_exit(void){
  thread_exit();
  return 0;
}

int sys_thread_join(void){
  int thread;
  
  // Extract the thread ID to join
  if(argint(0, &thread) < 0) {
    return -1;
  }
  
  return thread_join(thread);
}

int sys_waitpid(void)
{
  int pid;
  
  // Extract the process ID to wait for
  if(argint(0, &pid) < 0) {
    return -1;
  }
  
  return waitpid(pid);
}

int sys_sleepChan(void) {
  return -1;
}

int sys_getChannel(void) {
  return -1;
}

int sys_sigChan(void) {
  return -1;
}

int sys_sigOneChan(void) {
  return -1;
}

