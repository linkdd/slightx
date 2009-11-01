SRC =
OBJ = $(SRC:.c=.o)
EXEC = libc.a

.PHONY: all clean

all: $(EXEC)
	@mv $(EXEC) ../bin

clean:
	@rm -rf $(OBJ) $(EXEC)

$(EXEC): $(OBJ)
	@ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -Iinclude/ $^ -o $@
