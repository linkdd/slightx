#ifndef __SCREEN_H
#define __SCREEN_H

#define RAMSCREEN    0xC00B8000
#define SIZESCREEN   0xFA0
#define SCREENLIM    0xC00B8FA0

void clear (void);
void scrollup (int n);
void putchar (char c);
void printk (char *format, ...);

#endif /* __SCREEN_H */
