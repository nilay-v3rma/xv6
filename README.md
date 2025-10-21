# xv6 with Demand Paging

This branch implements **demand paging** for xv6 on ARM. Physical memory is allocated only when pages are actually accessed, not when virtual memory is allocated.

## How It Works

1. `sbrk()` calls use `allocuvm_demand()` - just updates process size, no physical allocation
2. When process accesses unmapped page → page fault 
3. `dabort_handler()` catches fault and calls `handle_page_fault()`
4. Physical memory allocated and mapped to virtual address
5. Process continues execution

## Key Changes

- **`vm.c`**: Added `allocuvm_demand()` and `handle_page_fault()`
- **`trap.c`**: Modified `dabort_handler()` to handle page faults  
- **`trap_asm.S`**: Fixed PC adjustment for ARM data abort return
- **`proc.c`**: Changed `growproc()` to use demand allocation
- **`copyuvm()`**: Skip unallocated pages during fork

## Benefits

- Memory only allocated when actually used
- Faster process creation (`sbrk()` is immediate)
- Better memory utilization

## ARM-Specific Details

- Detects translation faults using FSR codes 0x5 and 0x7
- Critical PC adjustment in `trap_dabort` for instruction retry
- Proper TLB flushing after new page mappings