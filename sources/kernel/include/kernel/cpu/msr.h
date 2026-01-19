#pragma once

#include <klibc/types.h>


#define IA32_FS_BASE        0xC0000100
#define IA32_GS_BASE        0xC0000101
#define IA32_KERNEL_GS_BASE 0xC0000102
#define IA32_APIC_BASE      0x1B


u64  rdmsr(u64 msr);
void wrmsr(u64 msr, u64 value);
