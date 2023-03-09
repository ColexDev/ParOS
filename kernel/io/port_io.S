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

global disable_blinking
disable_blinking:
	; Read I/O Address 0x03DA to reset index/data flip-flop
	mov dx, 0x03DA
	in al, dx
	; Write index 0x30 to 0x03C0 to set register index to 0x30
	mov dx, 0x03C0
	mov al, 0x30
	out dx, al
	; Read from 0x03C1 to get register contents
	inc dx
	in al, dx
	; Unset Bit 3 to disable Blink
	and al, 0xF7
	; Write to 0x03C0 to update register with changed value
	dec dx
	out dx, al
