#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct OPCODES{
    char mnemonic[20];
    char opcode[2];
    int format;
}opcodes[100];
struct symbolTable{
    char symbol[20];
    int address;
}symbols[100];
int opcodeCount = 0;
int symbolCount = 0;
void readOpcodes(FILE *opcodeFile) {
    char line[100];
    while (fgets(line, sizeof(line), opcodeFile) != NULL) {
        if (sscanf(line, "%s %s %d", opcodes[opcodeCount].mnemonic, opcodes[opcodeCount].opcode, &opcodes[opcodeCount].format) == 3) {
            opcodeCount++;
        } else {
            fprintf(stderr, "Invalid opcode format: %s\n", line);
        }
    }
}
int findFormat(char key[]){
    for(int i=0;i<opcodeCount;i++){
        if(strcmp(opcodes[i].mnemonic, key) == 0){
            return opcodes[i].format;
        }
    }
    if(key[0] == '+'){
        if(findFormat(key+1) == 3){
            return 4;
        } else{
            return -1;
        }
    }
    return -1;
}
int pass1(char code[], char opcode[], char intermediate[], char symbolTable[]) {
    char *LABEL, *OPCODE, *OPERAND, instruction[3][100], line[300];
    int instructionIndex = 0, STARTING_ADDRESS, LOCCTR;
    FILE *codeFile, *opcodeFile, *intermediateFile, *symbolTableFile;
    bool first = true;
    codeFile = fopen(code, "r");
    if (codeFile == NULL) {
        perror("Error opening code file");
        exit(EXIT_FAILURE);
    }
    opcodeFile = fopen(opcode, "r");
    if (opcodeFile == NULL) {
        perror("Error opening opCode file");
        exit(EXIT_FAILURE);
    }
    intermediateFile = fopen(intermediate, "w");
    if (intermediateFile == NULL) {
        perror("Error opening intermediate file");
        exit(EXIT_FAILURE);
    }
    symbolTableFile = fopen(symbolTable, "w");
    if (symbolTableFile == NULL) {
        perror("Error opening symbolTable file");
        exit(EXIT_FAILURE);
    }
    readOpcodes(opcodeFile);
    while (fgets(line, sizeof(line), codeFile) != NULL) {
        memset(instruction, 0, sizeof(instruction));
        instructionIndex=0;
        char *token = strtok(line, " \t\n");
        while (token !=NULL) {
            strcpy(instruction[instructionIndex++], token);
            token = strtok(NULL, " \t\n");
        }
        LABEL=OPCODE=OPERAND=NULL;
        if (instructionIndex == 3) {
            LABEL = instruction[0];
            OPCODE = instruction[1];
            OPERAND = instruction[2];
        } else if (instructionIndex == 2) {
            OPCODE = instruction[0];
            OPERAND = instruction[1];
        } else if (instructionIndex == 1) {
            OPCODE = instruction[0];
        }
        if (first) {
            if (strcmp(OPCODE, "START") == 0) {
                STARTING_ADDRESS = (int)strtol(OPERAND, NULL, 16);
                LOCCTR = STARTING_ADDRESS;
                fprintf(intermediateFile, "%04X %s %s %s\n", STARTING_ADDRESS, LABEL ? LABEL : "", OPCODE, OPERAND);
                first = false;
                continue;
            } else {
                fprintf(stderr, "Missing START directive\n");
                fclose(codeFile);
                fclose(opcodeFile);
                fclose(intermediateFile);
                fclose(symbolTableFile);
                return EXIT_FAILURE;
            }
        }
        // Handle end of program
        if (strcmp(OPCODE, "END") == 0) {
            fprintf(intermediateFile, "%04X %s %s %s\n", LOCCTR, LABEL ? LABEL : "", OPCODE, OPERAND ? OPERAND : "");
            break;
        }
        int instructionFormat=-1;
        if(strcmp(OPCODE, "BYTE") == 0){
            if (OPERAND[0] == 'C') {
                instructionFormat = strlen(OPERAND) - 3;
            } else if (OPERAND[0] == 'X') {
                instructionFormat = (strlen(OPERAND) - 3) / 2;
            } else {
                fprintf(stderr, "Invalid BYTE format: %s\n", OPERAND);
                return EXIT_FAILURE;
            }
        } else if(strcmp(OPCODE, "WORD") == 0){
            instructionFormat = 3;
        } else if(strcmp(OPCODE, "RESW") == 0){
            instructionFormat = 3 * atoi(OPERAND);
        } else if(strcmp(OPCODE, "RESB") == 0){
            instructionFormat = atoi(OPERAND);
        } else{
            // Determine instruction format and update LOCCTR
            instructionFormat = findFormat(OPCODE);
            if (instructionFormat == -1) {
                fprintf(stderr, "Invalid opcode: %s\n", OPCODE);
                fclose(codeFile);
                fclose(opcodeFile);
                fclose(intermediateFile);
                fclose(symbolTableFile);
                return EXIT_FAILURE;
            }
        }
        // Add label to symbol table if it exists
        if (LABEL) {
            for (int i = 0; i < symbolCount; i++) {
                if (strcmp(symbols[i].symbol, LABEL) == 0) {
                    fprintf(stderr, "Duplicate symbol: %s\n", LABEL);
                    continue;
                }
            }
            strcpy(symbols[symbolCount].symbol, LABEL);
            symbols[symbolCount++].address = LOCCTR;
        }
        // Write intermediate line
        fprintf(intermediateFile, "%04X %s %s %s\n", LOCCTR, LABEL ? LABEL : "", OPCODE, OPERAND ? OPERAND : "");
        LOCCTR += instructionFormat;
    }
    for(int i=0;i<symbolCount;i++){
        fprintf(symbolTableFile,"%s %X\n", symbols[i].symbol, symbols[i].address);
    }
    fclose(codeFile);
    fclose(opcodeFile);
    fclose(intermediateFile);
    fclose(symbolTableFile);
    return EXIT_SUCCESS;
}

int main() {
    pass1("code", "opcode", "intermediate", "symbolTable");
    return 0;
}
