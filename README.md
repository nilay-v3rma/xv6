This branch will be updated for the following tasks.

## 1. Barriers as synchronization primitives -> Implemented
In this part, we will implement barriers.

A barrier works as follows. The barrier is initialized with a count N, using the system call barrier_init(N). Next, processes that wish to wait at the barrier invoke the system call barrier_check(). The first N − 1 calls to barrier_check must block, and the N-th call to this function must unblock all the processes that were waiting at the barrier. That is, all the N processes that wish to synchronize at the barrier must cross the barrier only after all N of them have arrived. You must implement the core logic for these system calls in barrier.c.

You may assume that the barrier is used only once, so you need not worry about reinitializing it multiple times. You may also assume that all N processes will eventually arrive at the barrier.

You must implement the barrier synchronization logic using the sleep and wakeup primitives of the xv6 kernel, along with kernel spinlocks. Note that it does not make sense to use the userspace sleep/wakeup functionality developed by you in this part of the assignment, because the code for these system calls must be written within the kernel using kernel synchronization primitives.

To test your barrier implementation, the zip file contains a test program t_barrier.c. In this program, a parent and its two children arrive at the barrier at different times. With the skeleton code provided to you, all of them clear the barrier as soon as they enter. However, once you implement the barrier, you will find that the order of the print statements will change to reflect the fact that the processes block until all of them check into the barrier.

## 2. Threads
xv6 in its original form does not provide any primitives to create multiple threads within a process. In order to provide concurrency to users, we will implement three system calls in this part which essentially are all you need to use basic multithreading. In what follows, both tid and pid refer to the same quantity, the unique identity of a unit of execution.

- thread_create(uint*,void* (*func)(void*),void* arg): This is an analogue of the fork syscall which will be used to spawn threads. It returns the pid (or tid) allocated to the newly created thread. It takes as arguments:
    - a pointer to an integer in which the thread_id would be stored. For convenience, this value can be the same as the tid of the spawned thread
    - a function that takes a void* argument and has return type void*. This would be the entry point of the spawned thread
    - the void* argument passed to that function
- thread_exit(): This is an analogue of the exit syscall and is to be used exclusively by spawned threads. If this syscall is called from a main thread, the behaviour should be similar to a no-op.
- thread_join(uint): This is an analogue of the waitpid syscall and takes the thread_id of a thread as argument. Similar to thread_exit, this syscall is to be used exclusively to reap spawned threads.
Threads are fundamentally different from processes because in a way, they are linked to their main parent process. As a result, we need to keep track of the main parent thread inside each of the spawned threads. struct proc can be modified accordingly to contain new fields in this regard. We will also have to modify the behaviour of fork, exit and wait syscalls to acknowledge the presence of threads. Some instructions that may help:

- Threads share memory and hence page tables, so there is no need to create a new set of page tables everytime a thread is created.
- Forked processes are mainthreads themselves whereas spawned threads are not.
- Wait for the process to exit until the mainthread has exited
- Exit should only be called by the mainthread of a process
- Though threads share memory, they cannot share stacks. So, each thread would need a new writeable user page as its user stack. This would also mean that each time a new thread is spawned, the size of the process grows. You might run into trouble if you do not track this. For allocating a stack for the newly created thread, you need to create a new page (single page) for the stack, and give it user and write permissions, which can be done using kalloc or allocuvm. You will have to ensure that this allocation is visible to both the newly created thread and the process that created thread (as they share the pgdir).
- The spawned thread must start executing at the entrypoint function and not the next instruction. You must modify the EIP in the trapframe to achieve this.
- The void* argument being passed into the entrypoint function needs to be placed on the user stack so that the compiled code reads the correct data. You can look at how arguments can be put on the stack in the implementation of exec() syscall. You should put the arguments on the stack and decrease the stack pointer, and also remember that the stack pointer should point to the bottom of the stack page as the stack grows upwards.
- A number thread_id is to be assigned to the spawned thread — it is convenient to just use it as an alias for tid. This number then has to be copied into memory at the address passed as the first argument to this syscall.
- Mainthreads cannot be joined to themselves.
- Mainthreads can only join their spawned threads, not their children created using fork.
- Threads share memory, so one thread joining doesn’t necessarily mean that the entire memory address space needs to be freed up (as is done in waitpid). You might run into problems if you free memory prematurely. As a rule of thumb, memory does get freed when the mainthread gets reaped because we are ensuring that all the threads are also reaped along with it.
- There is still a portion of memory intended for a thread which can safely be deallocated regardless of memory address space being shared, this region of memory must be freed while joining.
A test case `t_threads.c` has been provided to you, where new threads update some shared variables. If thread creation works properly, the expected output will be as follows.

```
$ t_threads
Hello World
Thread 1 created 10
Thread 2 created 11
Value of x = 11
```

## 3. Userspace locks and condition variables
In this part, we will be implementing spinlocks in xv6, which will be used by the threads of a process to protect access to shared memory. 

Spinlocks can be implemented entirely in userspace using atomic instructions. Hence the implementation of spinlocks do not require you to add any new syscalls. All of your changes for implementing locks are to be done in user.h and ulib.c. You are expected to implement the following functionality in ulib.c:

- void initiateLock(struct lock* l): Initializes the lock by setting the value of l->lockvar to 0.
- void acquireLock(struct lock* l): Check if the lock has been initialized, if so, use the atomic xchg instruction to set l->lockvar to 1, thus acquiring the lock
- void releaseLock(struct lock* l): Check that the lock has been initialized and acquired, then set the value of l->lockvar back to 0.
You are provided testcase t_lock.c. The expected output after a correct implementation is shown below.

```
$ t_lock
Final value of x: 2000000
```

Next, we will implement conditional variables in xv6, to enable synchronization between the threads. Conditional variables, unlike spinlocks, require you to use syscalls to sleep and wake up processes. On top of some syscalls, you will build userspace APIs to initialize, wait, and signal condition variables.

Remember that waiting on a conditional variable and a lock involves releasing the lock, going to sleep on a particular channel, and reacquiring the lock on waking up. While we have made methods to acquire and release locks, we still lack a userspace API to sleep on a particular channel. Therefore, to correctly implement condition variables, you may want to first implement the following system calls, to expose sleep and wakeup APIs to user code.

- int getChannel(void): Should return the value of a channel that is unused and will remain unused. You must generate unique channels inside xv6 somehow, and the channel chosen should not clash with a channel that is being used, otherwise this will lead to spurious wakeups.
- void sleepChan(int): Should check that the provided argument is a valid channel value and then sleep on that channel value.
- void sigChan(int): Should check that the provided argument is a valid channel value and then wake up all processes sleeping on that channel value.
- void sigOneChan(int): Should check that the provided argument is a valid channel value and then wake up one specific process sleeping on that channel value.
Once the above syscalls are available to user code, you must implement condition variable functionality, which is already defined in user.h, by completing the following functions in ulib.c:

- void initiateCondVar(struct condvar* cv): Allocate a channel through getChannel and set cv->var to the channel’s value. Also track that the variable has been initialized.
- void condWait(struct condvar* cv, struct lock* l): Check that both the conditional variable and lock are initialized, then release the lock, sleep on the channel and re-acquire the lock after waking up.
- void broadcast(struct condvar* cv): Check that the conditional variable has been initialized and then call sigChan with appropriate arguments to wakeup all threads sleeping on the CV's channel.
- void signal(struct condvar* cv): Check that the conditional variable has been initialized and then call sigOneChan with appropriate arguments to wake up one particular thread sleeping on the CV's channel.
t_sleepwake.c, t_l_cv1.c and t_l_cv2.c are the test-cases. The expected output from the test cases is as follows.

```
$ t_sleepwake
going to sleep on channel 4
Woken up by child
$ t_l_cv1
Hello World
Gonna acquire lock
Acquired lock successfully
Value of x = 11
$ t_l_cv2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
I am thread 1
I am thread 2
```

## 4. Semaphores in xv6
In this part you will use the conditional variables and locks you implemented earlier to implement semaphores in xv6. Please solve this part only after you finish all the above parts.

The structure for the semaphore is given in user.h, it consists of a counter variable, a lock and a conditional variable, the implementation for the semaphore consists of three functions:

- void semInit(struct semaphore*, int initval): This function initializes the lock and conditional variable in the semaphore and sets the counter to initval.
- void semUp(struct semaphore* s): This function should acquire the lock, increment the counter, wake up exactly one process that is sleeping on the channel of the conditional variable.
- void semDown(struct semaphore* s): This function should acquire the lock, decrement the counter, if after decrementing, the counter becomes negative, it should call condWait on the conditional variable and the lock, then it should release the lock.
You are given testcases t_sem1.c, t_sem2.c. The expected output is as follows:

```
$ t_sem1
Hello World
Gonna acquire semaphore
Acquired semaphore successfully
Value of x = 11
$ t_sem2
I am thread3 and I should print first
I am thread 1
Only one other thread has printed by now
I am thread 2
Both the other threads have printed by now
```