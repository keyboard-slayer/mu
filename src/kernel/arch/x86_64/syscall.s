.extern syscall_handler
.globl syscall_handle

.macro __pusha
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
.endm

.macro __popa
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
.endm

syscall_handle:
    swapgs
    mov %rsp, %gs:0x8
    mov %gs:0x0, %rsp

    pushq $0x1b
    pushq %gs:0x8
    pushq %r11
    pushq $0x23
    pushq %rcx

    cld
    __pusha

    mov %rsp, %rdi
    mov $0, %rbp
    call syscall_handler

    __popa

    mov %gs:0x8, %rsp
    swapgs
    sysretq