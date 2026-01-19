#pragma once

#include <kernel/boot/int.h>


#define EXC_DIVIDE_BY_ZERO                0x0
#define EXC_DEBUG                         0x1
#define EXC_NON_MASKABLE_INTERRUPT        0x2
#define EXC_BREAKPOINT                    0x3
#define EXC_OVERFLOW                      0x4
#define EXC_BOUND_RANGE_EXCEEDED          0x5
#define EXC_INVALID_OPCODE                0x6
#define EXC_DEVICE_NOT_AVAILABLE          0x7
#define EXC_DOUBLE_FAULT                  0x8
#define EXC_COPROCESSOR_SEGMENT_OVERRUN   0x9
#define EXC_INVALID_TSS                   0xA
#define EXC_SEGMENT_NOT_PRESENT           0xB
#define EXC_STACK_SEGMENT_FAULT           0xC
#define EXC_GENERAL_PROTECTION_FAULT      0xD
#define EXC_PAGE_FAULT                    0xE
//reserved 0xF
#define EXC_X87_FLOATING_POINT            0x10
#define EXC_ALIGNMENT_CHECK               0x11
#define EXC_MACHINE_CHECK                 0x12
#define EXC_SIMD_FLOATING_POINT           0x13
#define EXC_VIRTUALIZATION                0x14
#define EXC_CONTROL_PROTECTION            0x15
//reserved 0x16-0x1B
#define EXC_HYPERVISOR_INJECTION          0x1C
#define EXC_VMM_COMMUNICATION             0x1D
#define EXC_SECURITY                      0x1E
// reserved 0x1F


void exc_init(void);

void exc_handler(interrupt_frame *iframe);

interrupt_cb exc_get_callback  (u8 exc);
void         exc_set_callback  (u8 exc, interrupt_cb cb);
void         exc_clear_callback(u8 exc);
