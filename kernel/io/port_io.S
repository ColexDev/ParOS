; uint8_t __attribute__((cdecl)) enable_interrupts();
global enable_interrupts
enable_interrupts:
    sti
    ret

; uint8_t __attribute__((cdecl)) disable_interrupts();
global disable_interrupts
disable_interrupts:
    cli
    ret

global crash_me
crash_me:
    ; div by 0
    mov eax, 0
    div eax

global kernel_panic
kernel_panic:
    cli
    hlt
