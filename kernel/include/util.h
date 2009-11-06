#ifndef __UTIL_H
#define __UTIL_H

void printk (char *format, ...);
int strlen (char *s);
int strncpy (char *dst, char *src, int n);
void itoa (char *buf, unsigned long int n, int base);

#endif /* __UTIL_H */
