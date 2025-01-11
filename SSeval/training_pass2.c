#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
struct OPCODES{
    char mnemonic[20];
    unsigned int opcode;
    int format;
}opcodes[100];
struct symbolTable{
    char symbol[20];
    unsigned int address;
}symbols[100];
int opcodeCount = 0;
int symbolCount = 0;
void readOpcodes(FILE *opcodeFile){
    char line[100];
    while(fgets(line, sizeof(line), opcodeFile)!=NULL){
        if(sscanf(line, "%s %X %d", opcodes[opcodeCount].mnemonic, &opcodes[opcodeCount].opcode, &opcodes[opcodeCount].format) == 3){
            opcodeCount++;
        } else{
            fprintf(stderr, "Invalid Opcode format : %s", line);
        }
    }
}
int findOpcode(char mnemonic[]){
    for(int i=0;i<opcodeCount;i++)
        if(strcmp(opcodes[i].mnemonic, mnemonic) == 0)
            return opcodes[i].opcode;
    if(mnemonic[0] == '+')
        return findOpcode(mnemonic+1);
    return -1;
}
int findFormat(char mnemonic[]){
    for(int i=0;i<opcodeCount;i++){
        if(strcmp(opcodes[i].mnemonic, mnemonic) == 0)
            return opcodes[i].format;
        if(mnemonic[0] == '+'){
            if(findFormat(mnemonic) == 3)
                return 4;
            else
                return -1;
        }
    }
    return -1;
}
void readSymbols(FILE *symbolTableFile){
    char line[100];
    while(fgets(line, sizeof(line), symbolTableFile)!=NULL){
        if(sscanf(line, "%s %X", symbols[symbolCount].symbol, &symbols[symbolCount].address) == 2){
            symbolCount++;
        } else{
            fprintf(stderr, "Invalid symbol format");
        }
    }
}
int findSymbol(char symbol[]){
    for(int i=0;i<symbolCount;i++){
        if(strcmp(symbols[i].symbol, symbol) == 0){
            return symbols[i].address;
        }
    }
    return -1;
}
bool checkX(char OPERAND[]){
    return strstr(OPERAND, ",X") || strstr(OPERAND, "X,");
}
int pass2(char intermediate1[], char opcode[], char symtab[], char intermediate2[], char objectCode[]){
    char *ADDRESS, *LABEL, *OPCODE, *OPERAND, instruction[4][25], line[100];
    FILE *intermediate1File, *opcodeFile, *symbolTableFile, *intermediate2File, *objectCodeFile, *tempObjectCodeFile;
    int textLength = 0, programLength = 0, instructionIndex = 0;
    long headerPosition = 0, textPosition = 0;
    intermediate1File = fopen(intermediate1, "r");
    if(intermediate1File == NULL){
        fprintf(stderr, "Error opening file : %s", intermediate1);
        return EXIT_FAILURE;
    }
    opcodeFile = fopen(opcode, "r");
    if(opcodeFile == NULL){
        fprintf(stderr, "Error opening file : %s", opcode);
        return EXIT_FAILURE;
    }
    symbolTableFile = fopen(symtab, "r");
    if(symbolTableFile == NULL){
        fprintf(stderr, "Error opening file : %s", symtab);
        return EXIT_FAILURE;
    }
    intermediate2File = fopen(intermediate2, "w");
    if(intermediate2File == NULL){
        fprintf(stderr, "Error opening file : %s", intermediate2);
        return EXIT_FAILURE;
    }
    objectCodeFile = fopen(objectCode, "w");
    if(objectCodeFile == NULL){
        fprintf(stderr, "Error opening file : %s", objectCode);
        return EXIT_FAILURE;
    }
    tempObjectCodeFile = fopen(objectCode, "w");
    readOpcodes(opcodeFile);
    readSymbols(symbolTableFile);
    while(fgets(line, sizeof(line), intermediate1File) != NULL){
        memset(instruction, 0, sizeof(instruction));
        instructionIndex = 0;
        char *token = strtok(line, " \t\n");
        while(token != NULL){
            strcpy(instruction[instructionIndex], token);
            token = strtok(NULL, " \t\n");
        }
        ADDRESS=LABEL=OPCODE=OPERAND=NULL;
        if(instructionIndex == 4){
            ADDRESS = instruction[0];
            LABEL = instruction[1];
            OPCODE = instruction[2];
            OPERAND = instruction[3];
        } else if(instructionIndex == 3){
            ADDRESS = instruction[0];
            OPCODE = instruction[1];
            OPERAND = instruction[2];
        } else if(instructionIndex == 2){
            ADDRESS = instruction[0];
            OPERAND = instruction[1];
        }
    }
    return EXIT_SUCCESS;
}
