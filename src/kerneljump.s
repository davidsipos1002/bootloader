.section .text

.global _KernelJumpStart, _KernelJumpEnd

.align 16
_KernelJumpStart:
    //Microsoft ABI
    //%rcx BootInfo
    //%rdx page_table
    //%r8 kernel_entry
    //System V ABI (Kernel ABI)
    //rdi first parameter is BootInfo struct
    cli
    mov %rdx, %cr3
    mov %rcx, %rdi
    jmp *%r8
    hlt
_KernelJumpEnd:
