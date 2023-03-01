#ifndef PIC_H
#define PIC_H

#include <stdint.h>

void pic_configure(uint8_t offset_pic1, uint8_t offset_pic2);
void pic_mask(int irq);
void pic_unmask(int irq);
void pic_send_EOI(int irq);
uint16_t pic_read_IRQ_register(void);
uint16_t pic_read_in_service_register(void);

#endif /* #ifndef PIC_H */
