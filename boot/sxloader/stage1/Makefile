.PHONY: all

all: stage1
	@mv stage1 ../../../bin/

stage1: stage1.asm
	$(AS) -f bin $^ -o $@
