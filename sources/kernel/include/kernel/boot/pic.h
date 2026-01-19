#pragma once

#include <klibc/types.h>


#define PIC_MASTER_OFFSET   0x20
#define PIC_SLAVE_OFFSET    0x28

#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1

#define PIC_EOI         0x20

#define ICW1_ICW4       0x01    // ICW4 (not) needed
#define ICW1_SINGLE     0x02    // Single (cascade) mode
#define ICW1_INTERVAL4  0x04    // Call address interval 4 (8)
#define ICW1_LEVEL      0x08    // Level triggered (edge) mode
#define ICW1_INIT       0x10    // Initialization - required!

#define ICW4_8086        0x01    // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO        0x02    // Auto (normal) EOI
#define ICW4_BUF_SLAVE   0x08    // Buffered mode/slave
#define ICW4_BUF_MASTER  0x0C    // Buffered mode/master
#define ICW4_SFNM        0x10    // Special fully nested (not)


void pic_load(void);

void pic_irq_set_mask  (u8 irq);
void pic_irq_clear_mask(u8 irq);
void pic_disable       (void);
void pic_remap         (u8 offset_master, u8 offset_slave);
void pic_eoi           (u8 irq);
