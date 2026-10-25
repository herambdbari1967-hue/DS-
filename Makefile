CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC = src/trie.c src/heap.c src/spellcheck.c src/dictionary.c src/main.c
OBJ = $(SRC:.c=.o)
TARGET = autocorrect

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe

standalone:
	$(CC) -Wall -O2 single_file_version/autocorrect_standalone.c -o autocorrect_standalone

.PHONY: all clean standalone
