[bits 64]

global idt_load
idt_load:

    ; Create new stack frame
    push rbp             ; Save old stack frame
    mov rbp, rsp         ; Initialize new stack frame
    
    ; Load idt
    mov eax, [rbp + 8]
    lidt [eax]

    ; Restore old stack frame
    mov rsp, rbp
    pop rbp
    ret
