#pragma once

#include <klibc/types.h>


#define IA32_FS_BASE        0xC0000100
#define IA32_GS_BASE        0xC0000101
#define IA32_KERNEL_GS_BASE 0xC0000102
#define IA32_APIC_BASE      0x1B

#define IA32_EFER           0xC0000080
#define IA32_STAR           0xC0000081
#define IA32_LSTAR          0xC0000082
#define IA32_FMASK          0xC0000084

#define IA32_EFER_SCE       (1ULL << 0)  // System Call Extensions


u64  rdmsr(u64 msr);
void wrmsr(u64 msr, u64 value);
