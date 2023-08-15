Things to do:
1. Setup 5-level page table
2. Memory map framebuffer
3. Write trampoline with symbols so that is doesn't occupy stack space
4. Memory map trampoline
5. Load ELF image in memory and map it
6. Exit boot services
7. Jump to kernel

kernel_main parameters:
    bootinfo - display info, framebuffer base and size
    pagetable - physical addresses for the 5 page tables