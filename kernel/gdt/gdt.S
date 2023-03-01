[bits 32]

global gdt_flush
extern gdt_ptr
gdt_flush:
    lgdt [gdt_ptr] ; Loads the GDT into the GDT register
    mov ax, 0x10     ; 0x10 is the offset into GDT data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush   ; 0x08 is the offset into GDT code segment

flush:
    ret
