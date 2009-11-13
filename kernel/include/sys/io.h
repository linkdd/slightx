#ifndef __IO_H
#define __IO_H

/*!
 * \file io.h
 * \brief I/O functions
 * \author David Delassus
 */

#include <types.h>

uint8_t inb (uint16_t port);
uint16_t inw (uint16_t port);
uint32_t ind (uint16_t port);

void outb (uint16_t port, uint8_t value);
void outw (uint16_t port, uint16_t value);
void outd (uint16_t port, uint32_t value);

void iowait (void);

#endif /* __IO_H */
