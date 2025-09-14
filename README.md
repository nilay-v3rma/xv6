This branch contains the base xv6 codebase with a modified scheduler from round robin to a boosted lottery scheduler.

### Implementation Details

- **Boosting Mechanism**:  
  To prevent starvation and improve responsiveness, a boosting mechanism is implemented. When a process voluntarily gives up cpu (like by going to sleep state or etc) its tickets are doubled (from the base tickets) for the number of ticks it was sleeping or not runnable.

- **Ticket Assignment**:  
  - Each process is initialized with a default number of tickets (1).
  - System call `settickets(pid, n)` is provided to allow dynamic adjustment of ticket counts by user programs or the kernel.
  - When a parent process forks and creates a child process, the child process inherits the parent's base tickets and no boost ticks.

- **Random Number Generation**:  
  The scheduler uses a pseudo-random number generator to select the winning ticket on each scheduling decision.

- **Code Changes**:  
  - Modifications are primarily in `proc.c` and `proc.h` to implement the lottery logic and ticket management.
  - Additional helper functions for ticket boosting and random selection are added.
  - The process structure (`struct proc`) is extended to include proper required attributes.

- **Testing**:  
  - Several test programs (in user level command `test` written in `usr/test.c`) are included to demonstrate the fairness and boosting behavior of the scheduler.
  - Edge cases, such as processes with zero tickets or maximum tickets, are handled gracefully.

- **How to Use**:  
  - Compile and run xv6 as usual -> `make clean qemu`.
  - Use provided test command or system calls to observe and modify ticket allocations.

---

That's all! <br>
Feel free to share any thoughts or feedback! :)