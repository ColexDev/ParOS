#include <io/port_io.h>
#include <stdint.h>

#include "pic.h"

/* 
 * Read this documentation to setup and disable the PIC
 * http://www.brokenthorn.com/Resources/OSDevPic.html 
*/

/* We will have 2 8259 PICs that are cascaded (slave is connected to
 * master on IRQ2) */

/* We will be disabling the PIC right away to use the APIC instead */
void
pic_remap(void)
{
    /* Calls to io_wait() may be necessary to give the PIC time to
     * respond to the corresponding commands */

    /* ICW1 (Initialization Control Word)
     * 0x11 = 0b10001:
     *  Bit 0 tells the PIC to expect ICW4
     *  Bit 4 is the initialization bit
    */
    outb(PIC1_COMMAND, 0x11);
    io_wait();
    outb(PIC2_COMMAND, 0x11);
    io_wait();

    /* ICW2
     * By default the master PIC (IRQ0-7) are mapped to interrupts
     * 0x08-0x0F. This overlaps with predefined/reserved x86 exception
     * vectors. We remap the master PIC (IRQ0-7) to interrupt 0x20 and
     * the slave PIC (IRQ8-15) to 0x28 to avoid these conflictions.
    */
    outb(PIC1_DATA, 0x20);
    io_wait();
    outb(PIC2_DATA, 0x28);
    io_wait();

    /* ICW3
     * This control word tells the master PIC what IRQ# the slave
     * is connected to. We also tell the slave what IRQ# the master
     * uses to connect. This lets them know how to communicate with eachother
     *
     * x86 uses IRQ2 on the master to connect the slave
     *
     * 0x04 = 0b100, this is the 2nd bit, meaning IRQ2
     * But then to the slave, we send the actual IRQ#, not the corresponding bit,
     * and as mentioned before, its IRQ2
    */
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    /* ICW4
     * Most bits in this ICW are unused on x86, we just need to set bit 0
     * which says we are in 8086 mode (because we are on x86!)
    */
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();
}


/* Mask all interrupts on both PICs */
void
pic_disable(void)
{
    outb(PIC1_DATA, 0xFF);
    io_wait();
    outb(PIC2_DATA, 0xFF);
    io_wait();
}
