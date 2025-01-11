#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

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
void readOpcodes(FILE *opcodeFile){
    char line[100];
    while(fgets(line, sizeof(line), opcodeFile)){
        if(sscanf(line, "%s %s %d", opcodes[opcodeCount].mnemonic, opcodes[opcodeCount].opcode, &opcodes[opcodeCount].format)==3){
            opcodeCount++;
        } else{
            fprintf(stderr, "INVALID OPCODE FORMAT : %s", line);
        }
    }
}
int findFormat(char mnemonic[]){
    for(int i=0;i<opcodeCount;i++){
        if(strcmp(opcodes[i].mnemonic, mnemonic)==0){
            return opcodes[i].format;
        }
        if(mnemonic[0] == '+'){
            if(findFormat(mnemonic+1)==3){
                return 4;
            } else{
                return -1;
            }
        }
    }
    return -1;
}
int pass1(char code[], char opcode[], char intermediate[], char symbolTable[]){
    char *LABEL, *OPCODE, *OPERAND, line[100], instruction[3][30];
    int instructionIndex = 0, STARTING_ADDRESS, LOCCTR;
    FILE *codeFile, *opcodeFile, *intermediateFile, *symbolTableFile;
    bool first = true;
    codeFile = fopen(code, "r");
    if(codeFile == NULL){
        fprintf(stderr, "Error opening file : %s", code);
        return EXIT_FAILURE;
    }
    opcodeFile = fopen(opcode, "r");
    if(opcodeFile == NULL){
        fprintf(stderr, "Error opening file : %s", opcode);
        return EXIT_FAILURE;
    }
    intermediateFile = fopen(intermediate, "r");
    if(intermediateFile == NULL){
        fprintf(stderr, "Error opening file : %s", intermediate);
        return EXIT_FAILURE;
    }
    symbolTableFile = fopen(symbolTable, "r");
    if(symbolTableFile == NULL){
        fprintf(stderr, "Error opening file : %s", symbolTable);
        return EXIT_FAILURE;
    }
    readOpcodes(opcodeFile);
    while(fgets(line, sizeof(line), codeFile) != NULL){
        memset(instruction, 0, sizeof(instruction));
        instructionIndex = 0;
        char *token = strtok(line, " \t\n");
        while(token != NULL){
            strcpy(instruction[instructionIndex++], token);
            token = strtok(NULL, " \t\n");
        }
        LABEL=OPCODE=OPERAND=NULL;
        if(instructionIndex==3){
            LABEL = instruction[0];
            OPCODE = instruction[1];
            OPERAND = instruction[2];
        } else if(instructionIndex == 2){
            OPCODE = instruction[0];
            OPERAND = instruction[1];
        } else if(instructionIndex == 1){
            OPCODE = instruction[0];
        }
        if(first){
            if(strcmp(OPCODE, "START") == 0){
                STARTING_ADDRESS = (int)strtol(OPERAND, NULL, 16);
                LOCCTR = STARTING_ADDRESS;
                fprintf(intermediateFile, "%04X %s %s %s\n", LOCCTR, LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
                first = false;
                continue;
            } else{
                fprintf(stderr, "Missing START directive");
                fclose(codeFile);
                fclose(opcodeFile);
                fclose(symbolTableFile);
                fclose(intermediateFile);
                return EXIT_FAILURE;
            }
        }
        if(strcmp(OPCODE, "END") == 0){
            fprintf(intermediateFile, "%04X %s %s %s\n", LOCCTR, LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
            break;
        }
        int instructionFormat = -1;
        if(strcmp(OPCODE, "BYTE") == 0){
            if(OPERAND[0] == 'C'){
                instructionFormat = strlen(OPERAND) - 3;
            } else if(OPERAND[0] == 'X'){
                instructionFormat = (strlen(OPERAND) - 3)/2;
            } else{
                fprintf(stderr, "Invalid Opcode : %s", OPERAND);
                return EXIT_FAILURE;
            }
        } else if(strcmp(OPCODE, "WORD") == 0){
            instructionFormat = 3;
        } else if(strcmp(OPCODE, "RESB") == 0){
            instructionFormat = atoi(OPERAND);
        } else if(strcmp(OPCODE, "RESW") == 0){
            instructionFormat = 3 * atoi(OPERAND);
        } else{
            instructionFormat = findFormat(OPCODE);
            if(instructionFormat == -1){
                fprintf(stderr, "Invalid Opcode");
                fclose(codeFile);
                fclose(opcodeFile);
                fclose(symbolTableFile);
                fclose(intermediateFile);
                return EXIT_FAILURE;
            }
        }
        if(LABEL){
            for(int i=0;i<symbolCount;i++){
                if(strcmp(symbols[i].symbol, LABEL) == 0){
                    fprintf(stderr, "Duplicate Symbol Found : %s\n", LABEL);
                    continue;
                }
            }
            strcpy(symbols[symbolCount].symbol, LABEL);
            symbols[symbolCount++].address = LOCCTR;
        }
        fprintf(intermediateFile, "%04X %s %s %s", LOCCTR, LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
        LOCCTR += instructionFormat;
    }
    for(int i=0;i<symbolCount;i++){
        fprintf(symbolTableFile, "%s %X", symbols[i].symbol, symbols[i].address);
    }
    fclose(codeFile);
    fclose(opcodeFile);
    fclose(symbolTableFile);
    fclose(intermediateFile);
    return EXIT_SUCCESS;
}
int main(){
    pass1("code", "opcode", "intermediate", "symbolTable");
}
