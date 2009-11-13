APPS_BIN = posix-test
KERNEL_BIN = kernel.bin
LIBC_BIN = libc.a
INITRD = initrd.img
.PHONY: install clean

install: make_initrd
	@if [ ! -d ../mnt/sys/ ] ; then mkdir ../mnt/sys/ ; fi
	@if [ ! -d ../mnt/bin/ ] ; then mkdir ../mnt/bin/ ; fi
	@cp -f $(KERNEL_BIN) $(LIBC_BIN) $(INITRD) ../mnt/sys/
	@cp -f $(APPS_BIN) ../mnt/bin/

make_initrd: make_initrd.c
	@$(CC) $^ -o $@
	@./make_initrd ../README README ../TODO TODO ../COPYING COPYING

clean:
	@rm -rf kernel.bin libc.a
