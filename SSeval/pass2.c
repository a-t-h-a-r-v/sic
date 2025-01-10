#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#define indirect_mask 2
#define immediate_mask 1
#define index_mask 8
#define base_mask 4
#define program_counter_mask 2
#define extended_mask 1
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
void readOpcodes(FILE *opcodeFile) {
    char line[100];
    while (fgets(line, sizeof(line), opcodeFile) != NULL) {
        if (sscanf(line, "%s %X %d", opcodes[opcodeCount].mnemonic, &opcodes[opcodeCount].opcode, &opcodes[opcodeCount].format) == 3) {
            opcodeCount++;
        } else {
            fprintf(stderr, "Invalid opcode format: %s\n", line);
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
void readSymbols(FILE *symbolTable) {
    char line[100];
    while (fgets(line, sizeof(line), symbolTable) != NULL) {
        if (sscanf(line, "%s %X", symbols[symbolCount].symbol, &symbols[symbolCount].address) == 2) {
            symbolCount++;
        } else {
            fprintf(stderr, "Invalid opcode format: %s\n", line);
        }
    }
}
int findSymbol(char symbol[]){
    for(int i=0;i<symbolCount;i++)
        if(strcmp(symbols[i].symbol, symbol) == 0)
            return symbols[i].address;
    return -1;
}
bool checkX(char OPERAND[]){
    return strstr(OPERAND, ",X") || strstr(OPERAND, "X,");
}
int pass2(char intermediate1[], char opcode[], char symtab[], char intermediate2[], char objectCode[]){
    char *ADDRESS, *LABEL, *OPCODE, *OPERAND, STARTING_ADDRESS[25], instruction[4][25], line[100];
    FILE *intermediate1File, *opcodeFile, *symbolTableFile, *intermediate2File, *objectCodeFile, *tempObjectCodeFile;
    int instructionIndex = 0,textLength = 0,programLength = 0;
    long headerPosition = 0,textPosition = 0;
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
        instructionIndex=0;
        char *token = strtok(line, " \t\n");
        while (token !=NULL) {
            strcpy(instruction[instructionIndex++], token);
            token = strtok(NULL, " \t\n");
        }
        ADDRESS=LABEL=OPCODE=OPERAND=NULL;
        if (instructionIndex == 4) {
            ADDRESS = instruction[0];
            LABEL = instruction[1];
            OPCODE = instruction[2];
            OPERAND = instruction[3];
        } else if (instructionIndex == 3) {
            ADDRESS = instruction[0];
            OPCODE = instruction[1];
            OPERAND = instruction[2];
        } else if (instructionIndex == 2) {
            ADDRESS = instruction[0];
            OPCODE = instruction[1];
        }
        int firstTwoDigits = findOpcode(OPCODE), instructionFormat = findFormat(OPCODE), thirdDigit = 0, twoFormatLastTwoDigits = 0, lastThreeDigits = 0, lastFiveDigits = 0;
        if(strcmp(OPCODE, "START") == 0){
            strcpy(STARTING_ADDRESS, ADDRESS);
            fprintf(objectCodeFile, "H^%-6s^%-6s^  ", LABEL, ADDRESS);
            headerPosition = ftell(objectCodeFile) - 2;
            fprintf(intermediate2File, "%s %s %s %s\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
        } else if(strcmp(OPCODE, "END") == 0){
            fprintf(intermediate2File, "%s %s %s %s\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
            fprintf(objectCodeFile, "\nE^%-6s", STARTING_ADDRESS);
        } else if (strcmp(OPCODE, "BYTE") == 0) {
            if(!text){
                fprintf(objectCodeFile, "\nT^%-6s^  ", ADDRESS);
                textPosition = ftell(objectCodeFile) -2;
                textLength = 0;
                text = true;
            }
            fprintf(objectCodeFile, "^");
            fprintf(intermediate2File, "%s %s %s %s ", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
            if (OPERAND[0] == 'C') {
                for (int i = 2; i < strlen(OPERAND) - 1; i++) {
                    fprintf(objectCodeFile, "%X", OPERAND[i]);
                    fprintf(intermediate2File, "%X", OPERAND[i]);
                }
                textLength+=strlen(OPERAND) - 3;
            } else if (OPERAND[0] == 'X') {
                for (int i = 2; i < strlen(OPERAND) - 1; i++) {
                    fprintf(objectCodeFile, "%c", OPERAND[i]);
                    fprintf(intermediate2File, "%c", OPERAND[i]);
                }
                textLength+=(strlen(OPERAND) - 3)*(1/2);
            } else {
                fprintf(stderr, "Invalid BYTE operand: %s\n", OPERAND);
                return EXIT_FAILURE;
            }
            fprintf(intermediate2File, "\n");
        } else if (strcmp(OPCODE, "WORD") == 0) {
            if(!text){
                fprintf(objectCodeFile, "\nT^%-6s^  ", ADDRESS);
                textPosition = ftell(objectCodeFile) -2;
                textLength = 0;
                text = true;
            }
            textLength+=3;
            int value = atoi(OPERAND);
            fprintf(objectCodeFile, "^%06X", value);
            fprintf(intermediate2File, "%s %s %s %s %06X\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "",value);
        } else if (strcmp(OPCODE, "RESW") == 0) {
            if(text){
                fflush(objectCodeFile);
                fseek(tempObjectCodeFile, textPosition, SEEK_SET);
                fprintf(tempObjectCodeFile, "%02X", textLength);
                programLength += textLength;
                text = false;
            }
            int reservedWords = atoi(OPERAND);
            fprintf(intermediate2File, "%s %s %s %s\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
        } else if (strcmp(OPCODE, "RESB") == 0) {
            if(text){
                fflush(objectCodeFile);
                fseek(tempObjectCodeFile, textPosition, SEEK_SET);
                fprintf(tempObjectCodeFile, "%02X", textLength);
                programLength += textLength;
                text = false;
            }
            fprintf(intermediate2File, "%s %s %s %s\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "");
        } else if(instructionFormat == 2){
            if(!text){
                fprintf(objectCodeFile, "\nT^%-6s^  ", ADDRESS);
                textPosition = ftell(objectCodeFile) -2;
                textLength = 0;
                text = true;
            }
            textLength += 2;
            switch (OPERAND[0]) {
                case 'A': break;
                case 'X': twoFormatLastTwoDigits += 10;
                    break;
                case 'L': twoFormatLastTwoDigits += 20;
                    break;
                case 'B': twoFormatLastTwoDigits += 30;
                    break;
                case 'S': twoFormatLastTwoDigits += 40;
                    break;
                case 'T': twoFormatLastTwoDigits += 50;
                    break;
                case 'F': twoFormatLastTwoDigits += 60;
                    break;
                default: fprintf(stderr, "INVALID REGISTERS : %s\n", OPERAND);
                    return EXIT_FAILURE;
            }
            if(strlen(OPERAND) > 1){
                switch (OPERAND[2]) {
                    case 'A': break;
                    case 'X': twoFormatLastTwoDigits += 1;
                              break;
                    case 'L': twoFormatLastTwoDigits += 2;
                              break;
                    case 'B': twoFormatLastTwoDigits += 3;
                              break;
                    case 'S': twoFormatLastTwoDigits += 4;
                              break;
                    case 'T': twoFormatLastTwoDigits += 5;
                              break;
                    case 'F': twoFormatLastTwoDigits += 6;
                              break;
                    default: fprintf(stderr, "INVALID REGISTERS : %s\n", OPERAND);
                             return EXIT_FAILURE;
                }
            }
            fprintf(objectCodeFile, "^%02X%02X", firstTwoDigits, twoFormatLastTwoDigits);
            fprintf(intermediate2File, "%s %s %s %s %02X%02X\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "", firstTwoDigits, twoFormatLastTwoDigits);
        } else if(instructionFormat == 3){
            if(!text){
                fprintf(objectCodeFile, "\nT^%-6s^  ", ADDRESS);
                textPosition = ftell(objectCodeFile) -2;
                textLength = 0;
                text = true;
            }
            textLength += 3;
            if(OPERAND[0] == '@'){
                firstTwoDigits = firstTwoDigits | indirect_mask;
                lastThreeDigits = findSymbol(OPERAND+1);
                if(lastThreeDigits == -1){
                    lastThreeDigits = atoi(OPERAND+1);
                }
            } else if(OPERAND[0] == '#'){ 
                firstTwoDigits = firstTwoDigits | immediate_mask;
                lastThreeDigits = findSymbol(OPERAND+1);
                if(lastThreeDigits == -1){
                    lastThreeDigits = atoi(OPERAND+1);
                }
            } else{
                firstTwoDigits = firstTwoDigits | immediate_mask;
                firstTwoDigits = firstTwoDigits | indirect_mask;
                thirdDigit = thirdDigit | program_counter_mask;
                lastThreeDigits = findSymbol(OPERAND);
                if(lastThreeDigits == -1){
                    fprintf(stderr, "INVALID SYMBOL : %s\n", OPERAND);
                    return EXIT_FAILURE;
                }
                lastThreeDigits = lastThreeDigits - ((int)strtol(ADDRESS, NULL, 16) + instructionFormat);
                lastThreeDigits = abs(lastThreeDigits);
                lastThreeDigits &= 0xFFF;
            }
            if(checkX(OPERAND))
                thirdDigit = thirdDigit | index_mask;
            fprintf(objectCodeFile, "^%02X%01X%03X", firstTwoDigits, thirdDigit, lastThreeDigits);
            fprintf(intermediate2File, "%s %s %s %s %02X%01X%03X\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "", firstTwoDigits, thirdDigit, lastThreeDigits);
        } else if(instructionFormat == 4){
            if(!text){
                fprintf(objectCodeFile, "\nT^%-6s^  ", ADDRESS);
                textPosition = ftell(objectCodeFile) -2;
                textLength = 0;
                text = true;
            }
            textLength += 4;
            if(OPERAND[0] == '@'){
                firstTwoDigits = firstTwoDigits | indirect_mask;
                lastFiveDigits = findSymbol(OPERAND+1);
            } else if(OPERAND[0] == '#'){
                firstTwoDigits = firstTwoDigits | immediate_mask;
                lastFiveDigits = findSymbol(OPERAND+1);
            }
            else{
                firstTwoDigits = firstTwoDigits | indirect_mask;
                firstTwoDigits = firstTwoDigits | immediate_mask;
                lastFiveDigits = findSymbol(OPERAND);
            }
            lastFiveDigits &= 0xFFFFF;
            thirdDigit = thirdDigit | extended_mask;
            if(checkX(OPERAND))
                thirdDigit = thirdDigit | index_mask;
            fprintf(objectCodeFile, "^%02X%01X%05X", firstTwoDigits, thirdDigit, lastFiveDigits);
            fprintf(intermediate2File, "%s %s %s %s %02X%01X%05X\n", ADDRESS ? ADDRESS : "", LABEL ? LABEL : "", OPCODE ? OPCODE : "", OPERAND ? OPERAND : "", firstTwoDigits, thirdDigit, lastFiveDigits);
        }
    }
    if(text){
        fflush(objectCodeFile);
        fseek(tempObjectCodeFile, textPosition, SEEK_SET);
        fprintf(tempObjectCodeFile, "%02X", textLength);
        programLength += textLength;
        text = false;
    }
    fflush(objectCodeFile);
    fseek(tempObjectCodeFile, headerPosition, SEEK_SET);
    fprintf(tempObjectCodeFile, "%02X", programLength);
    fclose(intermediate1File);
    fclose(opcodeFile);
    fclose(symbolTableFile);
    fclose(intermediate2File);
    fclose(objectCodeFile);
    fclose(tempObjectCodeFile);
    return 0;
}
int main(){
    pass2("intermediate", "opcode", "symbolTable", "intermediate2", "objectCode");
}
