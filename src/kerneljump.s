.section .text

.global _KernelJumpStart, _KernelJumpEnd

.align 16
_KernelJumpStart:
    //%rcx BootInfo
    //%rdx page_table
    //%r8 kernel_entry
    mov %rdx, %cr3
    mov %rcx, %rdi
    jmp *%r8
_KernelJumpEnd:
