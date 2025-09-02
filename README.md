# xv6-pi5 Documentation

## Overview
xv6-pi5 is a port of the MIT xv6 teaching operating system to the ARM architecture, with a focus on compatibility with the Raspberry Pi 5 platform. It provides a minimal Unix-like kernel, shell, file system, and basic user programs, serving as a hands-on resource for learning operating system fundamentals on ARM hardware.

## Repository Structure
| Path/Directory | Purpose |
|----------------|---------|
| `arm.c`, `arm.h` | ARM CPU initialization, context switching, MMU setup |
| `asm.S` | Assembly routines for low-level CPU and trap handling |
| `entry.S` | Kernel entry point and bootstrap code |
| `swtch.S` | Context switch (process switching) assembly routine |
| `trap_asm.S` | Trap and interrupt entry assembly |
| `mmu.h` | ARM MMU and paging definitions |
| `kernel.ld` | Linker script for ARM memory layout |
| `initcode.S` | Minimal user-mode program for system initialization |
| `device/` | Device drivers (UART, timer, interrupt controller, etc.) |
| `console.c` | Console (UART) driver and kernel I/O |
| `main.c`, `start.c` | Kernel initialization and main loop |
| `Makefile` | Build system configuration for ARM toolchain |
| `usr/` | User programs (e.g., sh, ls, cat, etc.) |
| `tools/` | Build utilities (e.g., mkfs for file system creation) |
| Other `.c`/`.h` | Core kernel subsystems (proc, vm, file system, etc.) |

## Getting Started

### Prerequisites
- **ARM GCC Toolchain**: `arm-none-eabi-gcc` (for ARMv6/ARMv7) or `aarch64-linux-gnu-gcc` (for ARMv8/AArch64, Pi 5).
- **QEMU**: For ARM system emulation and testing.
- **Make**: Standard build utility.

### Building xv6-pi5
1. **Clone the Repository**:
   ```bash
   git clone -b xv6-pi5 https://github.com/bobbysharma05/OS.git
   cd OS/src
   ```
2. **Build the Kernel and User Programs**:
   ```bash
   make clean
   make
   ```
3. **Run in QEMU**:
   ```bash
   qemu-system-arm -M versatilepb -m 128 -cpu arm1176 -nographic -kernel kernel.elf
   ```
   You should see the xv6 shell prompt: `$`

## Features
- **Minimal Unix-like Kernel**: Process management, virtual memory, system calls.
- **ARM Support**: All low-level CPU, trap, and MMU code adapted for ARM.
- **Shell and Userland**: Simple shell and standard Unix utilities (ls, cat, echo, etc.).
- **File System**: xv6-style file system with support for basic file operations.
- **UART Console**: Serial console for kernel and shell I/O.
- **QEMU Compatibility**: Easily testable in QEMU before deploying to hardware.

## Key ARM-Specific Components
- **CPU and MMU Initialization**: Implemented in `arm.c`, `arm.h`, `mmu.h`, and associated assembly files. Handles setting up the ARM page tables, enabling the MMU, and configuring CPU modes.
- **Trap and Interrupt Handling**: Assembly files (`asm.S`, `trap_asm.S`, `entry.S`) provide the trap vector and interrupt entry points. Kernel C code handles dispatch and processing.
- **UART/Console**: `console.c` and device drivers in `device/` configure and use the Raspberry Pi’s UART for boot and shell interaction.
- **Linker Script**: `kernel.ld` ensures the kernel is loaded at the correct physical address for ARM.

## Porting Notes
- **Architecture-Specific Files**: All files related to CPU initialization, assembly, MMU, and device drivers are ARM-specific and differ from the x86/RISC-V versions of xv6.
- **Build System**: The `Makefile` and build scripts are set up for ARM toolchains. Adjust toolchain paths if necessary for your environment.
- **Testing**: QEMU is used for initial bring-up. For real Raspberry Pi 5 hardware, further adaptation (especially for new peripherals) may be required.

## Usage Example
```bash
$ ls
.              1 1 512
..             1 1 512
cat            2 2 8620
echo           2 3 8340
grep           2 4 9528
init           2 5 8560
kill           2 6 8332
ln             2 7 8364
ls             2 8 9332
mkdir          2 9 8412
rm             2 10 8404
sh             2 11 13532
stressfs       2 12 8616
usertests      2 13 32956
wc             2 14 8904
zombie         2 15 8184
UNIX           2 16 7828
console        3 17 0
$
```
This demonstrates a successful boot, shell launch, and file system access.
