LDFLAGS = -melf_i386 -Ttext=40000000 --entry=main
.PHONY: all clean

all: posix-test
	@mv $^ ../bin

clean:

posix-test: posix-test.c
	$(CC) $^ -o $@
