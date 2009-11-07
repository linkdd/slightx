SRC_C = main.c screen.c mem/gdt.c mem/mem.c mem/idt.c interrupts.c sys/io.c sys/pic.c util.c cpu.c
SRC_AS = loader.asm ints.asm
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

