# SIC Assembler - Pass 1

## This is the Pass 1 of a SIC (Simplified Instructional Computer) assembler implemented in C. The program performs the first pass over the assembly code to generate the symbol table, detect labels, and calculate addresses.
## Features
    1. Reads SIC assembly code from an input file.
    2. Creates a symbol table with labels and their corresponding addresses.
    3. Calculates addresses for each instruction and stores them for Pass 2.
    4. Detects errors such as duplicate labels and invalid instructions.
## Project Structure
    .
    ├── src/
    │   ├── main.c         # Entry point for the assembler
    │   ├── sic.c          # Contains main functions for Pass 1
    │   └── sic.h          # Header file with function declarations and macros
    ├── include/
    │   └── sic.h          # Shared header file for the SIC program
    └── README.md          # Project documentation
