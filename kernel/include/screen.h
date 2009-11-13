#ifndef __SCREEN_H
#define __SCREEN_H

/*!
 * \file screen.h
 * \brief Function to print on screen
 * \author David Delassus
 */

#include <types.h>

#define RAMSCREEN    0xB8000
#define SIZESCREEN   0xFA0
#define SCREENLIM    0xB8FA0

void clear (void);
void scrollup (int n);
void putchar (char c);
void printk (char *format, ...);

#endif /* __SCREEN_H */
