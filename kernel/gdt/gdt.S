[bits 64]

global gdt_flush
extern gdt_ptr
gdt_flush:
    ; lgdt [gdt_ptr]
    ; mov rbx, rsp
    ; push 0x10
    ; push rbx
    ; pushf
    ; push 0x08
    ; push .l1
    ; iretq
    ; .l1:
