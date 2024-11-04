# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Iinclude -g

# Source and object files
SRC = src/main.c src/sic.c
OBJ = $(SRC:.c=.o)

# Output executable name
OUTPUT = sic

# Default target to build the program
all: $(OUTPUT)

# Rule to link object files into executable
$(OUTPUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUTPUT)

# Rule to compile each .c file into a .o file
src/%.o: src/%.c include/sic.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove compiled files
clean:
	rm -f $(OBJ) $(OUTPUT)
