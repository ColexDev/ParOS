global gdt_load

gdt_load:
    lgdt [rdi]

    ; Reload kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    pop rdi

    ; Tell retfq to set cs to 0x08 and jump to 
    ; address at rdi (reload kernel code segment)
    push 0x08
    push rdi

    retfq
