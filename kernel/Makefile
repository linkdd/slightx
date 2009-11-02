SRC_C = main.c screen.c gdt.c mem.c
SRC_AS = loader.asm
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

