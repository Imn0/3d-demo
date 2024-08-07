CC = clang
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS = -g 
# CFLAGS = -ggdb3 
LDFLAGS = -lm -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf

SRC  = main.c
OBJDIR = obj
OBJ  = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))

.PHONY: all clean

all: dirs lnk

dirs:
	mkdir -p $(OBJDIR) 

run: all
	./3d-demo

lnk: $(OBJ)
	$(CC) -o 3d-demo $^ $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(OBJDIR)
	rm 3d-demo

