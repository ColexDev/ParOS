#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry
{
    uint16_t base_low;
    uint16_t selector;

    uint8_t zero;
    uint8_t flags;

    uint16_t base_mid;
    uint32_t base_high;
    uint32_t pad;
} __attribute__((packed));

struct idt_pointer
{
	uint16_t  limit;
	uintptr_t base;
} __attribute__((packed));

struct regs
{
	/* Pushed by common stub */
	uintptr_t r15, r14, r13, r12;
	uintptr_t r11, r10, r9, r8;
	uintptr_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

	/* Pushed by irq_helper.S */
	uintptr_t int_no, err_code;

	/* Pushed by interrupt */
	uintptr_t rip, cs, rflags, rsp, ss;
};

typedef struct regs * (*interrupt_handler_t)(struct regs *);
typedef struct regs * (*isr_handler_t)(struct regs *);

void idt_init(void);
void isr_install(uint8_t index, isr_handler_t isr);

extern struct regs *_isr0(struct regs*);
extern struct regs *_isr1(struct regs*);
extern struct regs *_isr2(struct regs*);
extern struct regs *_isr3(struct regs*);
extern struct regs *_isr4(struct regs*);
extern struct regs *_isr5(struct regs*);
extern struct regs *_isr6(struct regs*);
extern struct regs *_isr7(struct regs*);
extern struct regs *_isr8(struct regs*);
extern struct regs *_isr9(struct regs*);
extern struct regs *_isr10(struct regs*);
extern struct regs *_isr11(struct regs*);
extern struct regs *_isr12(struct regs*);
extern struct regs *_isr13(struct regs*);
extern struct regs *_isr14(struct regs*);
extern struct regs *_isr15(struct regs*);
extern struct regs *_isr16(struct regs*);
extern struct regs *_isr17(struct regs*);
extern struct regs *_isr18(struct regs*);
extern struct regs *_isr19(struct regs*);
extern struct regs *_isr20(struct regs*);
extern struct regs *_isr21(struct regs*);
extern struct regs *_isr22(struct regs*);
extern struct regs *_isr23(struct regs*);
extern struct regs *_isr24(struct regs*);
extern struct regs *_isr25(struct regs*);
extern struct regs *_isr26(struct regs*);
extern struct regs *_isr27(struct regs*);
extern struct regs *_isr28(struct regs*);
extern struct regs *_isr29(struct regs*);
extern struct regs *_isr30(struct regs*);
extern struct regs *_isr31(struct regs*);
extern struct regs *_isr32(struct regs*); /* Timer interrupt */
extern struct regs *_isr33(struct regs*); /* PS2 Keyboard interrupt */
extern struct regs *_isr128(struct regs*); /* Syscall */

#endif
