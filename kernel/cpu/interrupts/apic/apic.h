#ifndef APIC_H
#define APIC_H

#include <stdint.h>

#define IOAPICID 0x00
#define IOAPICVER 0x01
#define IOAPICARB 0x02
/* Each redirection entry uses 2 addresses (e.g. 0x10/0x11) */
#define IOREDTBL(n) (0x10 + 2 * n)

void apic_init(void);

// union ioapic_redirection_entry
// {
//     struct fields
//     {
//         uint8_t vector;
//         uint8_t delivery_mode : 3;
//         uint8_t destination_mode : 1;
//         uint8_t delivery_status : 1;
//         uint8_t interrupt_polarity : 1;
//         uint8_t remote_irr : 1;
//         uint8_t trigger_mode : 1;
//         uint8_t interrupt_mask : 1;
//         uint64_t reserved : 39;
//         uint8_t destination;
//     };
//     uint64_t bits;
// };

#endif /* APIC_H */
