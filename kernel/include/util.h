#ifndef __UTIL_H
#define __UTIL_H

#include <stdarg.h>

#define PANIC(msg)      panic (__FILE__, __LINE__, msg);
#define ASSERT(b)       ((b) ? (void) 0 : panic_assert (__FILE__, __LINE__, #b))

void printk (char *format, ...);
void itoa (char *buf, unsigned long int n, int base);

void panic (const char *file, int line, char *msg);
void panic_assert (const char *file, int line, char *msg);

void memset (void *dest, char val, unsigned int len);
void memcpy (void *dest, const void *src, unsigned int len);

int strlen (char *s);
int strcmp (char *str1, char *str2);
char *strcpy (char *dest, const char *src);
char *strncpy (char *dst, char *src, int n);

#endif /* __UTIL_H */
