# Makefile for FreeX

export CC = gcc
export AS = nasm
export LD = ld

export CFLAGS = -fno-builtin -m32 -Wall -Werr -nostartfiles -nostdlib -g -c
export ASFLAGS = -f elf
export LDFLAGS = -melf_i386 -Ttext=100000 --entry=_start

.PHONY: all clean install run

all:
	@make all -C boot
	@make all -C kernel
	@make all -C libc

clean:
	@make clean -C boot
	@make clean -C kernel
	@make clean -C libc
	@rm -rf bin/*

install: all
	@echo "Not yet implemented"

run: install
	@echo "Not yet implemented"
