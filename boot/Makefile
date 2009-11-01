SRC = 
OBJ = $(SRC:.asm=.o)

EXEC = boot.bin

.PHONY: all clean

all: $(EXEC)
	@mv $(EXEC) ../bin

clean:
	@rm -rf $(OBJ) $(EXEC)

$(EXEC): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $^ -o $@
