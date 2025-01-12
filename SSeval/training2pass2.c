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
    char symbol[100];
    unsigned int address;
}symbols[100];
int symbolCount = 0;
int opcodeCount = 0;
void readOpcodes(FILE *opcodeFile){
    char line[100];
    while(fgets(line, sizeof(line), opcodeFile) != NULL){
        if(sscanf(line, "%s %X %X", opcodes[opcodeCount].mnemonic, &opcodes[opcodeCount].opcode, &opcodes[opcodeCount].format) == 3){
            opcodeCount++;
        } else{
            fprintf(stderr, "Invalid opcode format : %s", line);
        }
    }
}
int findOpcode(char mnemonic[]){
    for(int i=0;i<opcodeCount;i++){
        if(strcmp(opcodes[i].mnemonic, mnemonic) == 0)
            return opcodes[i].opcode;
        if(mnemonic[0] == '+')
            return findOpcode(mnemonic + 1);
    }
    return -1;
}
int findFormat(char mnemonic[]){
    for(int i=0;i<opcodeCount;i++){
        if(strcmp(opcodes[i].mnemonic, mnemonic) == 0)
            return opcodes[i].format;
        if(mnemonic[0] == '+'){
            if(findFormat(mnemonic + 1) == 3) return 4;
            else return -1;
        }
    }
    return -1;
}
void readSymbols(FILE *symbolTableFile){
    char line[100];
    while(fgets(line, sizeof(line), symbolTableFile) != NULL){
        if(sscanf(line, "%s %X", symbols[symbolCount].symbol, &symbols[symbolCount].address) == 2){
            symbolCount++;
        } else{
            fprintf(stderr, "Invalid Symbol format : %s\n", line);
        }
    }
}
int findSymbol(char symbol[]){
    for(int i=0;i<symbolCount;i++){
        if(strcmp(symbols[i].symbol, symbol) == 0)
            return symbols[i].address;
    }
    return -1;
}
bool checkX(char OPERAND[]){
    return strstr(OPERAND, ",X") || strstr(OPERAND, "X,");
}
int pass2(char intermediate1[], char opcode[], char symtab[], char intermediate2[], char objectCode[]){
    FILE *intermediate1File, *opcodeFile, *symbolTableFile, *intermediate2File, *objectCodeFile, *tempObjectCodeFile;
    char line[100], instruction[4][25], *ADDRESS, *LABEL, *OPCODE, *OPERAND, STARTING_ADDRESS[25];
    int textLength = 0, programLength = 0, instructionIndex;
    long textPosition = 0, headerPosition = 0;
    bool text = false;
    intermediate1File = fopen(intermediate1, "r");
    if (intermediate1File == NULL) {
        perror("Error opening intermediate1 file");
        exit(EXIT_FAILURE);
    }
    opcodeFile = fopen(opcode, "r");
    if (opcodeFile == NULL) {
        perror("Error opening opcode file");
        exit(EXIT_FAILURE);
    }
    symbolTableFile = fopen(symtab, "r");
    if (symbolTableFile == NULL) {
        perror("Error opening symtab file");
        exit(EXIT_FAILURE);
    }
    intermediate2File = fopen(intermediate2, "w");
    if (intermediate2File == NULL) {
        perror("Error opening intermediate2 file");
        exit(EXIT_FAILURE);
    }
    objectCodeFile = fopen(objectCode, "w");
    if (objectCodeFile == NULL) {
        perror("Error opening objectCode file");
        exit(EXIT_FAILURE);
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
        ADDRESS = LABEL = OPCODE = OPERAND = NULL;
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
            OPCODE = instruction[1];
        }
        int firstTwoDigits = findOpcode(OPCODE), instructionFormat = findFormat(OPCODE), thirdDigit = 0, twoFormatLastTwoDigits = 0, lastThreeDigits = 0, lastFiveDigits = 0;
        if(strcmp(OPCODE, "START") == 0){
            strcpy(STARTING_ADDRESS, ADDRESS);
            fprintf(objectCodeFile, "H^%-6s^%-6s^      ", LABEL ? LABEL : "", STARTING_ADDRESS);
            headerPosition = ftell(objectCodeFile) - 6;
            fprintf(intermediate2File, "%s %s %s %s", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
        } else if(strcmp(OPCODE, "END") == 0){
            fprintf(objectCodeFile, "E^%s", STARTING_ADDRESS);
            fprintf(intermediate2File, "%s %s %s %s", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
            break;
        } else if(strcmp(OPCODE, "BYTE") == 0){
            if(!text){
                fprintf(objectCodeFile, "T^%s^  ", ADDRESS);
                textPosition = ftell(objectCodeFile) - 2;
                textLength = 0;
                text = true;
            }
            fprintf(objectCodeFile, "^");
            fprintf(intermediate2File, "%s %s %s %s ", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
            if(OPERAND[0] == 'C'){
                for(int i=2;i<strlen(OPERAND)-1;i++){
                    fprintf(objectCodeFile, "%X", OPERAND[i]);
                    fprintf(intermediate2File, "%X", OPERAND[i]);
                }
                textLength += strlen(OPERAND) - 3;
            } else if(OPERAND[0] == 'X'){
                for(int i=2;i<strlen(OPERAND)-1;i++){
                    fprintf(objectCodeFile, "%c", OPERAND[i]);
                    fprintf(intermediate2File, "%c", OPERAND[i]);
                }
                textLength += (strlen(OPERAND) - 3)/2;
            } else{
                fprintf(stderr, "INVALID BYTE FORMAT : %s", OPERAND);
                return EXIT_FAILURE;
            }
            fprintf(intermediate2File, "\n");
        } else if(strcmp(OPCODE, "WORD") == 0){
            if(!text){
                fprintf(objectCodeFile, "T^%-6s^%-6s^      ", LABEL ? LABEL : "", ADDRESS ? ADDRESS : "");
                textPosition = ftell(objectCodeFile);
                textLength = 0;
                text = true;
            }
            textLength += 3;
            int value = atoi(OPERAND);
            fprintf(objectCodeFile, "^%06X", value);
            fprintf(intermediate2File, "%s %s %s %s %06X\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "", value);
        } else if(strcmp(OPCODE, "RESB") == 0){
            textLength += atoi(OPERAND);
            if(text){
                fflush(objectCodeFile);
                fseek(tempObjectCodeFile, textPosition, SEEK_SET);
                fprintf(tempObjectCodeFile, "%02X", textLength);
                programLength += textLength;
                text = false;
            }
            fprintf(intermediate2File, "%s %s %s %s ", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
        } else if(strcmp(OPCODE, "RESW") == 0){
            textLength += 3*atoi(OPERAND);
            if(text){
                fflush(objectCodeFile);
                fseek(tempObjectCodeFile, textPosition, SEEK_SET);
                fprintf(tempObjectCodeFile, "%02X", textLength);
                programLength += textLength;
                text = false;
            }
            fprintf(intermediate2File, "%s %s %s %s ", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
        }
    }
    return EXIT_SUCCESS;
}
