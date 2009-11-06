# Makefile for FreeX

export CC = gcc
export AS = nasm
export LD = ld

export CFLAGS = -fno-builtin -m32 -Wall -nostartfiles -nostdlib -nostdinc -g -c
export ASFLAGS = -f elf
export LDFLAGS = -melf_i386 -T linker.ld

.PHONY: all clean install run

all:
	@make all -C kernel
	@make all -C libc
	@make all -C apps

clean:
	@make clean -C kernel
	@make clean -C libc
	@make clean -C apps
	@make clean -C bin

install:
	@losetup -o 32256 /dev/loop0 disk.img
	@mount -t xfs /dev/loop0 mnt
	@make install -C boot
	@make install -C bin
	@umount mnt
	@losetup -d /dev/loop0

run:
	@qemu -hda disk.img -m 512
