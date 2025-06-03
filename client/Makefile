CC = gcc
CFLAGS = -Wall -Iinclude -lncurses
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = bin/maze_game

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses -lcurl

clean:
	rm -f src/*.o
	rm -f $(TARGET)