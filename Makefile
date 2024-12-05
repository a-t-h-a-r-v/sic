# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Iinclude -g

# Source and object files
SRC_PASS1 = src/pass1/main.c src/sic.c
SRC_PASS2 = src/pass2/main.c src/sic.c
SRC_PASS1_LOADER = src/pass1Loader/main.c src/sic.c

# Object files
OBJ_PASS1 = $(SRC_PASS1:.c=.o)
OBJ_PASS2 = $(SRC_PASS2:.c=.o)
OBJ_PASS1_LOADER = $(SRC_PASS1_LOADER:.c=.o)

# Output executable name
OUTPUT_PASS1 = pass1
OUTPUT_PASS2 = pass2
OUTPUT_PASS1_LOADER = pass1Loader

# Default target to build the program
all: $(OUTPUT_PASS1) $(OUTPUT_PASS2) $(OUTPUT_PASS1_LOADER)

# Target to build pass1
pass1: $(OUTPUT_PASS1)

# Target to build pass2
pass2: $(OUTPUT_PASS2)

# Target to build pass1 loader
pass1Loader: $(OUTPUT_PASS1_LOADER)

# Rule to link object files into executable for pass1
$(OUTPUT_PASS1): $(OBJ_PASS1)
	$(CC) $(OBJ_PASS1) -o $(OUTPUT_PASS1)

# Rule to link object files into executable for pass2
$(OUTPUT_PASS2): $(OBJ_PASS2)
	$(CC) $(OBJ_PASS2) -o $(OUTPUT_PASS2)

# Rule to link object files into executable for pass1
$(OUTPUT_PASS1_LOADER): $(OBJ_PASS1_LOADER)
	$(CC) $(OBJ_PASS1_LOADER) -o $(OUTPUT_PASS1_LOADER)

# Rule to compile each .c file into a .o file
src/%.o: src/%.c include/sic.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove compiled files
clean:
	rm -f $(OBJ_PASS1) $(OBJ_PASS2) $(OBJ_PASS1_LOADER) $(OUTPUT_PASS1) $(OUTPUT_PASS2) $(OUTPUT_PASS1_LOADER)
