#pragma once

#include <klibc/types.h>


void klog (const char *fmt, ...);
void vklog(const char *fmt, va_list ap);
