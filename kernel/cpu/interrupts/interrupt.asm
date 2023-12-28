extern interrupt_handler

global isr0

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
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Save current segment onto the stack
    mov ax, ds
    push rax

    mov ax, 0x10 ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Pass stack pointer to the C function.
    ; This is basically setting up a struct on the stack and passing
    ; the starting address (now the top of the stack) of it to the 
    ; C function (interrupt_handler). The interrupt_frame struct
    ; starts with the segment (ds) register because that is the
    ; last one pushed onto the stack (the top). So the SP points to
    ; the value of ds that was pushed onto the stack
    mov rdi, rsp

    ; Clear direction flag (functions expect this)
    cld

    call interrupt_handler

    ; Restores old segment 
    pop rax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Pop all general purpose registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
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

isr0:
    push byte 0
    push byte 0
    jmp common_interrupt_handler
