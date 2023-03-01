[bits 32]

global idt_load
idt_load:

    ; Create new stack frame
    push ebp             ; Save old stack frame
    mov ebp, esp         ; Initialize new stack frame
    
    ; Load idt
    mov eax, [ebp + 8]
    lidt [eax]

    ; Restore old stack frame
    mov esp, ebp
    pop ebp
    ret
