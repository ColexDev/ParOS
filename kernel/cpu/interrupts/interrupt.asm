extern interrupt_handler

global isr128

bits 64

common_interrupt_handler:
    ; Push all general purpose registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp

    ; Save current segment onto the stack
    mov ax, ds
    push rax

    mov ax, 0x10 ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call interrupt_handler

    ; Restores old segment 
    pop rax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Pop all general purpose registers
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Gets rid of error code and interrupt number from the stack
    add rsp, 16

    iretq

isr128:
    push 0
    push 128
    jmp common_interrupt_handler
