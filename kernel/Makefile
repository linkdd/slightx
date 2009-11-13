SRC_C = main.c screen.c util.c array.c sys/io.c mem/gdt.c sys/pic.c \
		mem/idt.c isr.c timer.c mem/paging.c mem/kheap.c task.c     \
		fs/initrd.c fs/fs.c sys/syscall.c
SRC_AS = loader.asm int.asm process.asm
OBJ = $(SRC_AS:.asm=.o) $(SRC_C:.c=.o)

EXEC = kernel.bin

.PHONY: all clean

all: $(EXEC)
	@mv $(EXEC) ../bin

clean:
	@rm -rf $(OBJ) $(EXEC)

$(EXEC): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -Iinclude/ $^ -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $^ -o $@

