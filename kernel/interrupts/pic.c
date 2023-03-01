#include "pic.h"
#include "../io/port_io.h"

#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT    0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT    0xA1

enum {
    PIC_ICW1_ICW4        = 0x01,
    PIC_ICW1_SINGLE      = 0x02,
    PIC_ICW1_INTERVAL4   = 0x04,
    PIC_ICW1_LEVEL       = 0x08,
    PIC_ICW1_INITIALIZE  = 0x10,
} PIC_ICW1;

enum {
    PIC_ICW4_8086 = 0x1,
    PIC_ICW4_AUTO_EOI = 0x2,
    PIC_ICW4_BUFFER_MASTER = 0x4,
    PIC_ICW4_BUFFER_SLAVE = 0x0,
    PIC_ICW4_BUFFERED = 0x8,
    PIC_ICW4_SFNM = 0x10,
} PIC_ICW4;

enum {
    PIC_CMD_EOI = 0x20,
    PIC_CMD_READ_IRR = 0x0A,
    PIC_CMD_READ_ISR = 0x0B,
} PIC_CMD;

void
pic_configure(uint8_t offset_pic1, uint8_t offset_pic2)
{
    /* Control word 1 */
    outb(PIC1_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    io_wait();
    outb(PIC2_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    io_wait();

    /* Control word 2 */
    outb(PIC1_DATA_PORT, offset_pic1);
    io_wait();
    outb(PIC2_DATA_PORT, offset_pic2);
    io_wait();

    /* Control word 3 */
    outb(PIC1_DATA_PORT, 0x4); /* Tell PIC1 that it has slave on IRQ2 (0x4) */
    io_wait();
    outb(PIC2_DATA_PORT, 0x2); /* Tell PIC2 its cascade identity (0x2) */
    io_wait();

    /* Control word 4 */
    outb(PIC1_DATA_PORT, PIC_ICW4_8086); /* Tell PIC1 that it has slave on IRQ2 (0x4) */
    io_wait();
    outb(PIC2_DATA_PORT, PIC_ICW4_8086); /* Tell PIC2 its cascade identity (0x4) */
    io_wait();

    /* Clear data registers */
    outb(PIC1_DATA_PORT, 0);
    io_wait();
    outb(PIC2_DATA_PORT, 0);
    io_wait();
}

void
pic_mask(int irq)
{
    if (irq > 8) {
        irq -= 8;
    }

    uint8_t mask = inb(PIC1_DATA_PORT);
    outb(PIC1_DATA_PORT, mask | (1 << irq));
}

void
pic_unmask(int irq)
{
    if (irq > 8) {
        irq -= 8;
    }

    uint8_t mask = inb(PIC1_DATA_PORT);
    outb(PIC1_DATA_PORT, mask & ~(1 << irq));
}

void
pic_disable(void)
{
    outb(PIC1_DATA_PORT, 0xFF);
    io_wait();
    outb(PIC2_DATA_PORT, 0xFF);
    io_wait();
}

void
pic_send_EOI(int irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND_PORT, PIC_CMD_EOI);
    }
    outb(PIC1_COMMAND_PORT, PIC_CMD_EOI);
}

uint16_t
pic_read_IRQ_register(void)
{
    outb(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    outb(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);
    return inb(PIC2_COMMAND_PORT) | (inb(PIC2_COMMAND_PORT) << 8);
}

uint16_t
pic_read_in_service_register(void)
{
    outb(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    outb(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);
    return inb(PIC2_COMMAND_PORT) | (inb(PIC2_COMMAND_PORT) << 8);
}
