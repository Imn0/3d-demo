CC = clang
# CC = gcc
CFLAGS = -std=gnu11 -O3 -Wall -Wextra
CFLAGS += -g 
# CFLAGS = -ggdb3 
LDFLAGS = -lm -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf

SRC  = main.c
OBJDIR = obj
OBJ  = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
BIN = bin
TARGET = 3d-demo

.PHONY: all clean run

all: $(TARGET)

run: $(TARGET)
	./3d-demo

$(TARGET): $(OBJ) | $(BIN)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	@mkdir -p $(@D)
	$(CC) -o $@ -c $< $(CFLAGS)

$(BIN):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)