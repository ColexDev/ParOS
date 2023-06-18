.section .text
.align 4

/* Pushes a dummy error code for the stack structure to remain same */
.macro ISR_NOERR index
    .global _isr\index
    .type _isr\index, @function
    _isr\index:
        pushq $0x00
        pushq $\index
        jmp isr_common
.endm

/* Pushes the interrupt number */
.macro ISR_ERR index
    .global _isr\index
    .type _isr\index, @function
    _isr\index:
        pushq $\index
        jmp isr_common
.endm

ISR_NOERR 0  /* Division error */
ISR_NOERR 1  /* Debug */
ISR_NOERR 2  /* NMI */
ISR_NOERR 3  /* Breakpoint */
ISR_NOERR 4  /* Overflow */
ISR_NOERR 5  /* Bound range exceeded */
ISR_NOERR 6  /* Invalid opcode */
ISR_NOERR 7  /* Device not available */
ISR_ERR   8  /* Double fault */
ISR_NOERR 9  /* Coprosessor segment overrun */
ISR_ERR   10 /* Invalid TSS */
ISR_ERR   11 /* Segment not present */
ISR_ERR   12 /* Stack-segment fault */
ISR_ERR   13 /* General protection fault */
ISR_ERR   14 /* Page fault */
ISR_NOERR 15 /* Reserved */
ISR_NOERR 16 /* Floating point exception */
ISR_NOERR 17 /* Alignment check */
ISR_NOERR 18 /* Machine check */
ISR_NOERR 19 /* SIMD Floating point exception */
ISR_NOERR 20 /* Virtualization exception */
ISR_NOERR 21 /* Control protection exception */
ISR_NOERR 22 /* Reserved */
ISR_NOERR 23 /* Reserved */
ISR_NOERR 24 /* Reserved */
ISR_NOERR 25 /* Reserved */
ISR_NOERR 26 /* Reserved */
ISR_NOERR 27 /* Reserved */
ISR_NOERR 28 /* Hypervisor injection exception */
ISR_NOERR 29 /* mmu Communication exception */
ISR_NOERR 30 /* Security exception */
ISR_NOERR 31 /* Reserved */

ISR_NOERR 32 /* IRQs are just the same as an exception */
ISR_NOERR 33 /* PS2 Keyboard irq */

/* Syscall */
ISR_NOERR 128

.extern isr_handler
.type isr_handler, @function

.global isr_common
isr_common:
    /* Save all registers */
    swapgs
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

    cld

    /* Call the interrupt handler */
    mov %rsp, %rdi
    call isr_handler
    mov %rax, %rsp

    /* Restore all registers */
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
    pop %rax
    swapgs
    
    /* Cleanup error code and interrupt number */
    add $16, %rsp

    /* Return from interrupt */
    iretq

