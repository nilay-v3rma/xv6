This branch will be updated to include the implementation of demand-paging.

TODO:
1. handle page fault
2. implement fifo for physical page allocation

vm.c
- the `alloc_page` function handles allocating physical memories to virtual pages
- `mappages` handles mapping pte(s) to the allocated physical memory

approaches for demand paging
1. Invalid PTE initially

```
allocuvm() {
    for (each page) {
        pte = walkpgdir(pgdir, va, 1);  // Create PTE
        *pte = 0;  // Mark as invalid (no PE_TYPES bits set)
    }
}

// On page fault:
// 1. PTE exists but is invalid
// 2. Allocate physical memory
// 3. Update PTE to point to physical memory
```

2. Special Marking

```
allocuvm() {
    for (each page) {
        mappages(pgdir, va, 0, AP_KU);  // Physical address = 0
        // PTE has type bits set but physical address is 0
    }
}

// On page fault:
// 1. PTE exists and has type bits
// 2. But physical address is 0 (invalid)
// 3. Allocate physical memory  
// 4. Update PTE with real physical address
```

Changes so far
1. implemented `allocuvm_demand` to handle process size growth from `grow_proc` in `proc.c`