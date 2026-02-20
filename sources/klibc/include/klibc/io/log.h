#pragma once

#include <klibc/types.h>


void klogger_init(void);

void klog (const char *fmt, ...);
void vklog(const char *fmt, va_list ap);
