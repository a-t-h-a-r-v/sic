#include "sic.h"
int pass1(const char* codeFileName, const char* outputFileName, const char* opcodeFileName, const char* symtabFileName, const char* objectCodeFileName, const char* delimeter){
    int numOfLines = 0, LOCCTR, STARTINGADDRESS, numOps, instructionLength;
    char codeLine[3][20], *OPCODE, *OPERAND, *LABEL, *temp, *PROGRAMNAME = "";
    FILE *codeStream, *outputStream, *opcodeStream, *symtabStream, *objectCodeStream;
    SYMTAB* symbols = NULL;
    OPTAB* opcodes = NULL;

    codeStream = fopen(codeFileName, "r");
    if(codeStream == NULL){
        perror("Error opening input file");
        return 1;
    }
    outputStream = fopen(outputFileName, "w");
    if(outputStream == NULL){
        perror("Error opening input file");
        return 1;
    }
    opcodeStream = fopen(opcodeFileName, "r");
    if(opcodeStream == NULL){
        perror("Error opening input file");
        return 1;
    }
    symtabStream = fopen(symtabFileName, "w");
    if(symtabStream == NULL){
        perror("Error opening input file");
        return 1;
    }

    objectCodeStream = fopen(objectCodeFileName, "w");
    if(objectCodeStream == NULL){
        perror("Error opening input file");
        return 1;
    }

    readOpcodes(&opcodes, opcodeStream);
    fprintf(objectCodeStream, "H^");

    temp = readNextLine(codeStream);
    numOps = splitCodeLine(temp, codeLine, delimeter, 3);
    if((numOps == 2) && (strcmp(codeLine[0], "START") == 0)){
        OPCODE = codeLine[0];
        OPERAND = codeLine[1];
        STARTINGADDRESS = (int)strtol(OPERAND, NULL, 16);
        LOCCTR = STARTINGADDRESS;
        fprintf(outputStream, "%04X %s %s\n", LOCCTR, OPCODE, OPERAND);
        numOfLines++;
    }
    else if((numOps == 3) && (strcmp(codeLine[1], "START") == 0)){
        OPCODE = codeLine[1];
        OPERAND = codeLine[2];
        LABEL = codeLine[0];
        PROGRAMNAME = codeLine[0];
        STARTINGADDRESS = (int)strtol(OPERAND, NULL, 16);
        fprintf(objectCodeStream, "%s", PROGRAMNAME);
        LOCCTR = STARTINGADDRESS;
        fprintf(outputStream, "%04X %s %s %s\n", LOCCTR, LABEL, OPCODE, OPERAND);
        numOfLines++;
    }
    else{
        STARTINGADDRESS = 0;
        LOCCTR = 0;
        if(numOps == 1){
            OPCODE = codeLine[0];
            OPERAND = NULL;
            LABEL = NULL;
            fprintf(outputStream, "%04X %s\n", LOCCTR, OPCODE);
        }
        if(numOps == 2){
            OPCODE = codeLine[0];
            OPERAND = codeLine[1];
            LABEL = NULL;
            fprintf(outputStream, "%04X %s %s\n", LOCCTR, OPCODE, OPERAND);
        }

        else if(numOps == 3){
            LABEL = codeLine[0];
            OPCODE = codeLine[1];
            OPERAND = codeLine[2];
            fprintf(outputStream, "%04X %s %s %s\n", LOCCTR, LABEL, OPCODE, OPERAND);
        }
        numOfLines++;
        instructionLength = findInstructionLength(opcodes, OPCODE);
        if(instructionLength != -1){
            LOCCTR += instructionLength;
        }
        else if(strcmp(OPCODE, "WORD") == 0){
            LOCCTR += 3;
        }
        else if(strcmp(OPCODE, "RESW") == 0){
            LOCCTR += 3 * atoi(OPERAND);
        }
        else if(strcmp(OPCODE, "RESB") == 0){
            LOCCTR += atoi(OPERAND);
        }
        else if(strcmp(OPCODE, "BYTE") == 0){
            LOCCTR += strlen(OPERAND) -3;
        }
        else if(strcmp(OPCODE, "BASE") == 0){}
        else if(OPCODE[0] == '+'){
            char *token = strtok(OPCODE, "+");
            instructionLength = -1;
            if(token != NULL){
                instructionLength = findInstructionLength(opcodes, token);
            }
            if(instructionLength == 3){
                LOCCTR += 4;
            }
            else{
                printf("INVALID OPCODE : %s", OPCODE);
                return 1;
            }
        }
        else{
            printf("INVALID OPERATION CODE : %s", OPCODE);
            return 1;
        }
    }
    temp = readNextLine(codeStream);
    while(checkComment(temp)){
        temp = readNextLine(codeStream);
    }
    numOps = splitCodeLine(temp, codeLine, delimeter, 3);

    if(numOps == 2){
        OPCODE = codeLine[0];
        OPERAND = codeLine[1];
        LABEL = NULL;
    }

    else if(numOps == 3){
        LABEL = codeLine[0];
        OPCODE = codeLine[1];
        OPERAND = codeLine[2];
    }

    while((strcmp(OPCODE, "END") != 0) && temp != NULL && (numOps == 2 || numOps == 3)){
        if(LABEL != NULL){
            if(strcmp(OPCODE, "BASE") == 0){
                fprintf(outputStream, "%s %s %s\n", LABEL, OPCODE, OPERAND);
            }
            else{
                fprintf(outputStream, "%04X %s %s %s\n", LOCCTR, LABEL, OPCODE, OPERAND);
            }
        }
        else{
            if(strcmp(OPCODE, "BASE") == 0){
                fprintf(outputStream, "%s %s\n", OPCODE, OPERAND);
            }
            else{
                fprintf(outputStream, "%04X %s %s\n", LOCCTR, OPCODE, OPERAND);
            }
        }
        numOfLines++;
        if(LABEL != NULL){
            if(findSymbol(symbols, LABEL)){
                printf("DUPLICATE SYMBOL FOUND : %s\n", LABEL);
                return 1;
            }
            else{
                insertSymbol(&symbols, LABEL, LOCCTR);
            }
        }
        temp = readNextLine(codeStream);
        while(checkComment(temp)){
            temp = readNextLine(codeStream);
        }

        instructionLength = findInstructionLength(opcodes, OPCODE);
        if(instructionLength != -1){
            LOCCTR += instructionLength;
        }
        else if(strcmp(OPCODE, "WORD") == 0){
            LOCCTR += 3;
        }
        else if(strcmp(OPCODE, "RESW") == 0){
            LOCCTR += 3 * atoi(OPERAND);
        }
        else if(strcmp(OPCODE, "RESB") == 0){
            LOCCTR += atoi(OPERAND);
        }
        else if(strcmp(OPCODE, "BYTE") == 0){
            LOCCTR += strlen(OPERAND) -3;
        }
        else if(strcmp(OPCODE, "BASE") == 0){}
        else{
            printf("INVALID OPERATION CODE : %s", OPCODE);
            return 1;
        }
        numOps = splitCodeLine(temp, codeLine, delimeter, 3);

        if(numOps == 2){
            OPCODE = codeLine[0];
            OPERAND = codeLine[1];
            LABEL = NULL;
        }

        else if(numOps == 3){
            LABEL = codeLine[0];
            OPCODE = codeLine[1];
            OPERAND = codeLine[2];
        }
    }
    if(LABEL != NULL){
        if(strcmp(OPCODE, "BASE") == 0){
            fprintf(outputStream, "%s %s %s\n", LABEL, OPCODE, OPERAND);
        }
        else{
            fprintf(outputStream, "%04X %s %s %s\n", LOCCTR, LABEL, OPCODE, OPERAND);
        }
    }
    else{
        if(strcmp(OPCODE, "BASE") == 0){
            fprintf(outputStream, "%s %s\n", OPCODE, OPERAND);
        }
        else{
            fprintf(outputStream, "%04X %s %s\n", LOCCTR, OPCODE, OPERAND);
        }
    }
    writeSymtabToFile(symtabStream, symbols);

    printf("%d lines written in %s \n", numOfLines, outputFileName);
    printf("Program Length : %d\n", LOCCTR - STARTINGADDRESS);
    fprintf(objectCodeStream, "^%04X^%04X", STARTINGADDRESS, LOCCTR - STARTINGADDRESS);

    fclose(codeStream);
    fclose(outputStream);
    fclose(opcodeStream);
    return 1;
}

int pass2(const char* intermediateFileName, const char* symtabFileName, const char *opcodeFileName, const char* outputFileName, const char* objectCodeFileName, const char* delimeter){
    FILE *intermediateStream, *symtabStream, *outputStream, *objectCodeStream, *opcodeStream, *textLengthPointer;
    char *temp, intermediateLine[4][20], *LABEL, *OPCODE, *OPERAND, *ADDRESS, STARTINGADDRESS[20] = "0";
    int numOps = 0, textFlag = 0, textLength = 0;
    long textPosition;
    OPTAB *opcodes = NULL;
    SYMTAB *symbols = NULL;
    intermediateStream = fopen(intermediateFileName, "r");
    if(intermediateStream == NULL){
        perror("Error opening intermediate file");
        return 1;
    }

    symtabStream = fopen(symtabFileName, "r");
    if(symtabStream == NULL){
        perror("Error opening symtab file");
        return 1;
    }

    outputStream = fopen(outputFileName, "w");
    if(outputStream == NULL){
        perror("Error opening output file");
        return 1;
    }

    objectCodeStream = fopen(objectCodeFileName, "a");
    if(objectCodeStream == NULL){
        perror("Error opening object code file");
        return 1;
    }

    opcodeStream = fopen(opcodeFileName, "r");
    if(opcodeStream == NULL){
        perror("Error opening opcode file");
        return 1;
    }

    readOpcodes(&opcodes, opcodeStream);
    readSymtab(&symbols, symtabStream);
    temp = readNextLine(intermediateStream);
    numOps = splitCodeLine(temp, intermediateLine, delimeter, 4);
    if((numOps == 3) && (strcmp(intermediateLine[1], "START") == 0)){
        fprintf(outputStream, "%s %s %s\n", intermediateLine[0], intermediateLine[1], intermediateLine[2]);
        strcpy(STARTINGADDRESS, intermediateLine[2]);
    }
    else if((numOps == 4) && (strcmp(intermediateLine[2], "START") == 0)){
        fprintf(outputStream, "%s %s %s %s\n", intermediateLine[0], intermediateLine[1], intermediateLine[2], intermediateLine[3]);
        strcpy(STARTINGADDRESS, intermediateLine[3]);
    }
    else{
        if(strcmp(intermediateLine[0], "BASE") == 0){}
        else if(numOps == 3){
            ADDRESS = intermediateLine[0];
            LABEL = NULL;
            OPCODE = intermediateLine[1];
            OPERAND = intermediateLine[2];
            int instructionLength = findInstructionLength(opcodes, OPCODE);
            if(strcmp(OPCODE, "RESB") == 0){
                fprintf(outputStream, "%s %s %s\n", ADDRESS, OPCODE, OPERAND);
                if(textFlag != 0){
                    fflush(objectCodeStream);
                    textLengthPointer = fopen(objectCodeFileName, "r+");
                    fseek(textLengthPointer, textPosition, SEEK_SET);
                    fprintf(textLengthPointer, "%02X", textLength);
                    textPosition = 0;
                }
                textFlag = 0;
                textLength = 0;
            }
            else if(strcmp(OPCODE, "RESW") == 0){
                fprintf(outputStream, "%s %s %s\n", ADDRESS, OPCODE, OPERAND);
                if(textFlag != 0){
                    fflush(objectCodeStream);
                    textLengthPointer = fopen(objectCodeFileName, "r+");
                    fseek(textLengthPointer, textPosition, SEEK_SET);
                    fprintf(textLengthPointer, "%02X", textLength);
                    textPosition = 0;
                }
                textFlag = 0;
                textLength = 0;
            }
            else if(strcmp(OPCODE, "BYTE") == 0){
                fprintf(outputStream, "%s %s %s ", ADDRESS, OPCODE, OPERAND);
                if((OPERAND[0] == 'C') && (OPERAND[1] == 39) && (OPERAND[strlen(OPERAND) - 1] == 39)){
                    if(textFlag == 0){
                        textFlag = 1;
                        fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                        textPosition = ftell(objectCodeStream);
                        fprintf(objectCodeStream, "  ");
                    }
                    for(int i=2;i<strlen(OPERAND)-1;i++){
                        fprintf(outputStream, "%02X", OPERAND[i]);
                        fprintf(objectCodeStream, "%02X", OPERAND[i]);
                        textLength ++;
                    }
                    fprintf(outputStream, "\n");
                    if(textLength > 60){
                        if(textFlag != 0){
                            fflush(objectCodeStream);
                            textLengthPointer = fopen(objectCodeFileName, "r+");
                            fseek(textLengthPointer, textPosition, SEEK_SET);
                            fprintf(textLengthPointer, "%02X", textLength);
                            textPosition = 0;
                            textFlag = 0;
                            textLength = 0;
                        }
                    }
                }
                else if((OPERAND[0] == 'X') && (OPERAND[1] == 39) && (OPERAND[strlen(OPERAND) - 1] == 39)){
                    if(textFlag == 0){
                        textFlag = 1;
                        fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                        textPosition = ftell(objectCodeStream);
                        fprintf(objectCodeStream, "  ");
                    }
                    int tempHex = hex_to_int(OPERAND);
                    fprintf(outputStream, "%02X", tempHex);
                    fprintf(objectCodeStream, "%02X", tempHex);
                    textLength += 3;
                    fprintf(outputStream, "\n");
                    if(textLength > 60){
                        if(textFlag != 0){
                            fflush(objectCodeStream);
                            textLengthPointer = fopen(objectCodeFileName, "r+");
                            fseek(textLengthPointer, textPosition, SEEK_SET);
                            fprintf(textLengthPointer, "%02X", textLength);
                            textPosition = 0;
                            textFlag = 0;
                            textLength = 0;
                        }
                    }
                }
            }
            else if(strcmp(OPCODE, "WORD") == 0){
                fprintf(outputStream, "%s %s %s %06X\n", ADDRESS, OPCODE, OPERAND, atoi(OPERAND));
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%06X", atoi(OPERAND));
                textLength += 3;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 2){
                int firstReg = 0;
                int secondReg = 0;
                if(OPERAND[1] == ','){
                    if(strlen(OPERAND) == 3){
                        if(((firstReg = findRegValue(OPERAND[0])) != -1)){}
                        else{
                            printf("INVALID REGISTER : %c", OPERAND[0]);
                            return 1;
                        }
                        if(((secondReg = findRegValue(OPERAND[2])) != -1)){}
                        else{
                            printf("INVALID REGISTER : %c", OPERAND[2]);
                            return 1;
                        }
                    }
                }
                else if(strlen(OPERAND) == 1){
                    if(((firstReg = findRegValue(OPERAND[0])) != -1)){}
                    else{
                        printf("INVALID REGISTER : %c", OPERAND[0]);
                        return 1;
                    }
                }
                else{
                    printf("INVALID 2 ADDRESS FORMAT");
                    return 1;
                }
                fprintf(outputStream, "%s %s %s %s%d%d\n", ADDRESS, OPCODE, OPERAND, findOpcode(opcodes, OPCODE), firstReg, secondReg);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%s%d%d", findOpcode(opcodes, OPCODE), firstReg, secondReg);
                textLength += 2;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 3){
                char *currentOpcode = findOpcode(opcodes, OPCODE);
                currentOpcode[2] = '0';
                int firstThreeBits = (int)strtol(currentOpcode, NULL, 16) & 0xFFF;
                int lastThreeBits = 0;
                fprintf(outputStream, "%s %s %s", ADDRESS, OPCODE, OPERAND);
                if(OPERAND[0] == '@'){
                    firstThreeBits = firstThreeBits | indirect_mask;
                    char *token = strtok(OPERAND, "@");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(OPERAND[0] == '#'){
                    firstThreeBits = firstThreeBits | immediate_mask;
                    char *token = strtok(OPERAND, "#");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(checkX(OPERAND)){
                    firstThreeBits = firstThreeBits | index_mask;
                }
                if(OPERAND[0] != '#' && OPERAND[0] != '@'){
                    firstThreeBits = firstThreeBits | program_counter_mask;
                    firstThreeBits = firstThreeBits | indirect_mask;
                    firstThreeBits = firstThreeBits | immediate_mask;
                    lastThreeBits = findSymbolAddress(symbols, OPERAND) - (int)strtol(ADDRESS, NULL, 16) - instructionLength;
                    if(lastThreeBits < 0){
                        lastThreeBits &= 0xFFF;
                    }
                }
                fprintf(outputStream, " %03X%03X\n", firstThreeBits, lastThreeBits);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%03X%03X", firstThreeBits, lastThreeBits);
                textLength += 3;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 4){
                char *currentOpcode = findOpcode(opcodes, strtok(OPCODE, "+"));
                currentOpcode[2] = '0';
                int firstThreeBits = (int)strtol(currentOpcode, NULL, 16) & 0xFFF;
                int lastThreeBits = 0;
                firstThreeBits = firstThreeBits | extended_mask;
                fprintf(outputStream, "%s %s %s", ADDRESS, OPCODE, OPERAND);
                if(OPERAND[0] == '@'){
                    firstThreeBits = firstThreeBits | indirect_mask;
                    char *token = strtok(OPERAND, "@");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(OPERAND[0] == '#'){
                    firstThreeBits = firstThreeBits | immediate_mask;
                    char *token = strtok(OPERAND, "#");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(checkX(OPERAND)){
                    firstThreeBits = firstThreeBits | index_mask;
                }
                if(OPERAND[0] != '#' && OPERAND[0] != '@'){
                    firstThreeBits = firstThreeBits | program_counter_mask;
                    firstThreeBits = firstThreeBits | indirect_mask;
                    firstThreeBits = firstThreeBits | immediate_mask;
                    lastThreeBits = findSymbolAddress(symbols, OPERAND);
                }
                fprintf(outputStream, " %03X%05X\n", firstThreeBits, lastThreeBits);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%03X%05X", firstThreeBits, lastThreeBits);
                textLength += 4;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
        }
        else if(numOps == 4){
            ADDRESS = intermediateLine[0];
            LABEL = intermediateLine[1];
            OPCODE = intermediateLine[2];
            OPERAND = intermediateLine[3];
            int instructionLength = findInstructionLength(opcodes, OPCODE);
            if(strcmp(intermediateLine[2], "RESB") == 0){
                fprintf(outputStream, "%s %s %s %s\n", ADDRESS, LABEL, OPCODE, OPERAND);
                if(textFlag != 0){
                    fflush(objectCodeStream);
                    textLengthPointer = fopen(objectCodeFileName, "r+");
                    fseek(textLengthPointer, textPosition, SEEK_SET);
                    fprintf(textLengthPointer, "%02X", textLength);
                    textPosition = 0;
                }
                textFlag = 0;
                textLength = 0;
            }
            if(strcmp(intermediateLine[2], "RESW") == 0){
                fprintf(outputStream, "%s %s %s %s\n", ADDRESS, LABEL, OPCODE, OPERAND);
                if(textFlag != 0){
                    fflush(objectCodeStream);
                    textLengthPointer = fopen(objectCodeFileName, "r+");
                    fseek(textLengthPointer, textPosition, SEEK_SET);
                    fprintf(textLengthPointer, "%02X", textLength);
                    textPosition = 0;
                }
                textFlag = 0;
                textLength = 0;
            }
            if(strcmp(intermediateLine[2], "BYTE") == 0){
                fprintf(outputStream, "%s %s %s %s ", ADDRESS, LABEL, OPCODE, OPERAND);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ^");
                }
                if((OPERAND[0] == 'C') && (OPERAND[1] == 39) && (OPERAND[strlen(OPERAND) - 1] == 39)){
                    for(int i=2;i<strlen(OPERAND)-1;i++){
                        fprintf(outputStream, "%02X", OPERAND[i]);
                        fprintf(objectCodeStream, "%02X", OPERAND[i]);
                        textLength++;
                    }
                    fprintf(outputStream, "\n");
                    if(textLength > 60){
                        if(textFlag != 0){
                            fflush(objectCodeStream);
                            textLengthPointer = fopen(objectCodeFileName, "r+");
                            fseek(textLengthPointer, textPosition, SEEK_SET);
                            fprintf(textLengthPointer, "%02X", textLength);
                            textPosition = 0;
                            textFlag = 0;
                            textLength = 0;
                        }
                    }
                }
                else if((OPERAND[0] == 'X') && (OPERAND[1] == 39) && (OPERAND[strlen(OPERAND) - 1] == 39)){
                    if(textFlag == 0){
                        textFlag = 1;
                        fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                        textPosition = ftell(objectCodeStream);
                        fprintf(objectCodeStream, "  ");
                    }
                    int tempHex = hex_to_int(OPERAND);
                    fprintf(outputStream, "%02X", tempHex);
                    fprintf(objectCodeStream, "%02X", tempHex);
                    textLength += 3;
                    fprintf(outputStream, "\n");
                    if(textLength > 60){
                        if(textFlag != 0){
                            fflush(objectCodeStream);
                            textLengthPointer = fopen(objectCodeFileName, "r+");
                            fseek(textLengthPointer, textPosition, SEEK_SET);
                            fprintf(textLengthPointer, "%02X", textLength);
                            textPosition = 0;
                            textFlag = 0;
                            textLength = 0;
                        }
                    }
                }
            }
            if(strcmp(intermediateLine[2], "WORD") == 0){
                fprintf(outputStream, "%s %s %s %06X\n", ADDRESS, OPCODE, OPERAND, atoi(OPERAND));
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%06X",atoi(OPERAND));
                textLength += 3;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            if(instructionLength == 2){
                int firstReg = 0;
                int secondReg = 0;
                if(OPERAND[1] == ','){
                    if(strlen(OPERAND) == 3){
                        if(((firstReg = findRegValue(OPERAND[0])) != -1)){}
                        else{
                            printf("INVALID REGISTER : %c", OPERAND[0]);
                            return 1;
                        }
                        if(((secondReg = findRegValue(OPERAND[2])) != -1)){}
                        else{
                            printf("INVALID REGISTER : %c", OPERAND[2]);
                            return 1;
                        }
                    }
                }
                else if(strlen(OPERAND) == 1){
                    if(((firstReg = findRegValue(OPERAND[0])) != -1)){}
                    else{
                        printf("INVALID REGISTER : %c", OPERAND[0]);
                        return 1;
                    }
                }
                else{
                    printf("INVALID 2 ADDRESS FORMAT");
                    return 1;
                }
                fprintf(outputStream, "%s %s %s %s %s%d%d\n", ADDRESS, LABEL, OPCODE, OPERAND, findOpcode(opcodes, OPCODE), firstReg, secondReg);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%06X\n",atoi(OPERAND));
                textLength += 2;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 3){
                char *currentOpcode = findOpcode(opcodes, OPCODE);
                currentOpcode[2] = '0';
                int firstThreeBits = (int)strtol(currentOpcode, NULL, 16) & 0xFFF;
                int lastThreeBits = 0;
                fprintf(outputStream, "%s %s %s %s", ADDRESS, LABEL, OPCODE, OPERAND);
                if(OPERAND[0] == '@'){
                    firstThreeBits = firstThreeBits | indirect_mask;
                }
                else if(OPERAND[0] == '#'){
                    firstThreeBits = firstThreeBits | immediate_mask;
                    char *token = strtok(OPERAND, "#");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(checkX(OPERAND)){
                    firstThreeBits = firstThreeBits | index_mask;
                }
                if(OPERAND[0] != '#' && OPERAND[0] != '@'){
                    firstThreeBits = firstThreeBits | program_counter_mask;
                    firstThreeBits = firstThreeBits | indirect_mask;
                    firstThreeBits = firstThreeBits | immediate_mask;
                    lastThreeBits = findSymbolAddress(symbols, OPERAND) - (int)strtol(ADDRESS, NULL, 16) - instructionLength;
                    if(lastThreeBits < 0){
                        lastThreeBits &= 0xFFF;
                    }
                }
                fprintf(outputStream, " %03X%03X\n", firstThreeBits, lastThreeBits);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^", ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(outputStream, "^%03X%03X", firstThreeBits, lastThreeBits);
                textLength += 3;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 4){
                char *currentOpcode = findOpcode(opcodes, strtok(OPCODE, "+"));
                currentOpcode[2] = '0';
                int firstThreeBits = (int)strtol(currentOpcode, NULL, 16) & 0xFFF;
                int lastThreeBits = 0;
                firstThreeBits = firstThreeBits | extended_mask;
                fprintf(outputStream, "%s %s %s %s", ADDRESS, LABEL, OPCODE, OPERAND);
                if(OPERAND[0] == '@'){
                    firstThreeBits = firstThreeBits | indirect_mask;
                    char *token = strtok(OPERAND, "@");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(OPERAND[0] == '#'){
                    firstThreeBits = firstThreeBits | immediate_mask;
                    char *token = strtok(OPERAND, "#");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(checkX(OPERAND)){
                    firstThreeBits = firstThreeBits | index_mask;
                }
                if(OPERAND[0] != '#' && OPERAND[0] != '@'){
                    firstThreeBits = firstThreeBits | program_counter_mask;
                    firstThreeBits = firstThreeBits | indirect_mask;
                    firstThreeBits = firstThreeBits | immediate_mask;
                    lastThreeBits = findSymbolAddress(symbols, OPERAND);
                }
                fprintf(outputStream, " %03X%05X\n", firstThreeBits, lastThreeBits);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^", ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%03X%05X", firstThreeBits, lastThreeBits);
                textLength += 4;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
        }
    }
    temp = readNextLine(intermediateStream);
    numOps = splitCodeLine(temp, intermediateLine, delimeter, 4);
    if(numOps == 3){
        ADDRESS = intermediateLine[0];
        LABEL = NULL;
        OPCODE = intermediateLine[1];
        OPERAND = intermediateLine[2];
    }
    else if(numOps == 4){
        ADDRESS = intermediateLine[0];
        LABEL = intermediateLine[1];
        OPCODE = intermediateLine[2];
        OPERAND = intermediateLine[3];
    }
    while((strcmp(OPCODE, "END") != 0) && temp != NULL && (numOps == 3 || numOps == 4)){
        if(ADDRESS != NULL){
        }
        if(strcmp(intermediateLine[0], "BASE") == 0){}
        else if(numOps == 3){
            ADDRESS = intermediateLine[0];
            LABEL = NULL;
            OPCODE = intermediateLine[1];
            OPERAND = intermediateLine[2];
            int instructionLength = findInstructionLength(opcodes, OPCODE);
            if(strcmp(intermediateLine[1], "RESB") == 0){
                fprintf(outputStream, "%s %s %s\n", ADDRESS, OPCODE, OPERAND);
                if(textFlag != 0){
                    fflush(objectCodeStream);
                    textLengthPointer = fopen(objectCodeFileName, "r+");
                    fseek(textLengthPointer, textPosition, SEEK_SET);
                    fprintf(textLengthPointer, "%02X", textLength);
                    textPosition = 0;
                }
                textFlag = 0;
                textLength = 0;
            }
            if(strcmp(intermediateLine[1], "RESW") == 0){
                fprintf(outputStream, "%s %s %s\n", ADDRESS, OPCODE, OPERAND);
                if(textFlag != 0){
                    fflush(objectCodeStream);
                    textLengthPointer = fopen(objectCodeFileName, "r+");
                    fseek(textLengthPointer, textPosition, SEEK_SET);
                    fprintf(textLengthPointer, "%02X", textLength);
                    textPosition = 0;
                }
                textFlag = 0;
                textLength = 0;
            }
            if(strcmp(intermediateLine[1], "BYTE") == 0){
                fprintf(outputStream, "%s %s %s ", ADDRESS, OPCODE, OPERAND);
                if((OPERAND[0] == 'C') && (OPERAND[1] == 39) && (OPERAND[strlen(OPERAND) - 1] == 39)){
                    if(textFlag == 0){
                        textFlag = 1;
                        fprintf(objectCodeStream, "\nT^%s^", ADDRESS);
                        textPosition = ftell(objectCodeStream);
                        fprintf(objectCodeStream, "  ^");
                    }
                    for(int i=2;i<strlen(OPERAND)-1;i++){
                        fprintf(outputStream, "%02X", OPERAND[i]);
                        fprintf(objectCodeStream, "%02X", OPERAND[i]);
                        textLength++;
                    }
                    fprintf(outputStream, "\n");
                    if(textLength > 60){
                        if(textFlag != 0){
                            fflush(objectCodeStream);
                            textLengthPointer = fopen(objectCodeFileName, "r+");
                            fseek(textLengthPointer, textPosition, SEEK_SET);
                            fprintf(textLengthPointer, "%02X", textLength);
                            textPosition = 0;
                            textFlag = 0;
                            textLength = 0;
                        }
                    }
                }
                else if((OPERAND[0] == 'X') && (OPERAND[1] == 39) && (OPERAND[strlen(OPERAND) - 1] == 39)){
                    if(textFlag == 0){
                        textFlag = 1;
                        fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                        textPosition = ftell(objectCodeStream);
                        fprintf(objectCodeStream, "  ");
                    }
                    int tempHex = hex_to_int(OPERAND);
                    fprintf(outputStream, "%02X", tempHex);
                    fprintf(objectCodeStream, "%02X", tempHex);
                    textLength += 3;
                    fprintf(outputStream, "\n");
                    if(textLength > 60){
                        if(textFlag != 0){
                            fflush(objectCodeStream);
                            textLengthPointer = fopen(objectCodeFileName, "r+");
                            fseek(textLengthPointer, textPosition, SEEK_SET);
                            fprintf(textLengthPointer, "%02X", textLength);
                            textPosition = 0;
                            textFlag = 0;
                            textLength = 0;
                        }
                    }
                }
            }
            if(strcmp(intermediateLine[1], "WORD") == 0){
                fprintf(outputStream, "%s %s %s %06X\n", ADDRESS, OPCODE, OPERAND, atoi(OPERAND));
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%06X", atoi(OPERAND));
                textLength += 3;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            if(instructionLength == 2){
                int firstReg = 0;
                int secondReg = 0;
                if(OPERAND[1] == ','){
                    if(strlen(OPERAND) == 3){
                        if(((firstReg = findRegValue(OPERAND[0])) != -1)){}
                        else{
                            printf("INVALID REGISTER : %c", OPERAND[0]);
                            return 1;
                        }
                        if(((secondReg = findRegValue(OPERAND[2])) != -1)){}
                        else{
                            printf("INVALID REGISTER : %c", OPERAND[2]);
                            return 1;
                        }
                    }
                }
                else if(strlen(OPERAND) == 1){
                    if(((firstReg = findRegValue(OPERAND[0])) != -1)){}
                    else{
                        printf("INVALID REGISTER : %c", OPERAND[0]);
                        return 1;
                    }
                }
                else{
                    printf("INVALID 2 ADDRESS FORMAT");
                    return 1;
                }
                fprintf(outputStream, "%s %s %s %s%d%d\n", ADDRESS, OPCODE, OPERAND, findOpcode(opcodes, OPCODE), firstReg, secondReg);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^", ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%s%d%d", findOpcode(opcodes, OPCODE), firstReg, secondReg);
                textLength += 2;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 3){
                char *currentOpcode = findOpcode(opcodes, OPCODE);
                currentOpcode[2] = '0';
                int firstThreeBits = (int)strtol(currentOpcode, NULL, 16) & 0xFFF;
                int lastThreeBits = 0;
                fprintf(outputStream, "%s %s %s", ADDRESS, OPCODE, OPERAND);
                if(OPERAND[0] == '@'){
                    firstThreeBits = firstThreeBits | indirect_mask;
                }
                else if(OPERAND[0] == '#'){
                    firstThreeBits = firstThreeBits | immediate_mask;
                    char *token = strtok(OPERAND, "#");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(checkX(OPERAND)){
                    firstThreeBits = firstThreeBits | index_mask;
                }
                if(OPERAND[0] != '#' && OPERAND[0] != '@'){
                    firstThreeBits = firstThreeBits | program_counter_mask;
                    firstThreeBits = firstThreeBits | indirect_mask;
                    firstThreeBits = firstThreeBits | immediate_mask;
                    lastThreeBits = findSymbolAddress(symbols, OPERAND) - (int)strtol(ADDRESS, NULL, 16) - instructionLength;
                    if(lastThreeBits < 0){
                        //lastThreeBits *= -1;
                        lastThreeBits &= 0xFFF;
                    }
                }
                fprintf(outputStream, " %03X%03X\n", firstThreeBits, lastThreeBits);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^", ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%03X%03X", firstThreeBits, lastThreeBits);
                textLength += 3;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 4){
                char *currentOpcode = findOpcode(opcodes, strtok(OPCODE, "+"));
                currentOpcode[2] = '0';
                int firstThreeBits = (int)strtol(currentOpcode, NULL, 16) & 0xFFF;
                int lastThreeBits = 0;
                firstThreeBits = firstThreeBits | extended_mask;
                fprintf(outputStream, "%s %s %s", ADDRESS, OPCODE, OPERAND);
                if(OPERAND[0] == '@'){
                    firstThreeBits = firstThreeBits | indirect_mask;
                    char *token = strtok(OPERAND, "@");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(OPERAND[0] == '#'){
                    firstThreeBits = firstThreeBits | immediate_mask;
                    char *token = strtok(OPERAND, "#");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(checkX(OPERAND)){
                    firstThreeBits = firstThreeBits | index_mask;
                }
                if(OPERAND[0] != '#' && OPERAND[0] != '@'){
                    firstThreeBits = firstThreeBits | program_counter_mask;
                    firstThreeBits = firstThreeBits | indirect_mask;
                    firstThreeBits = firstThreeBits | immediate_mask;
                    lastThreeBits = findSymbolAddress(symbols, OPERAND);
                }
                fprintf(outputStream, " %03X%05X\n", firstThreeBits, lastThreeBits);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^", ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%03X%05X", firstThreeBits, lastThreeBits);
                textLength += 4;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
        }
        else if(numOps == 4){
            ADDRESS = intermediateLine[0];
            LABEL = intermediateLine[1];
            OPCODE = intermediateLine[2];
            OPERAND = intermediateLine[3];
            int instructionLength = findInstructionLength(opcodes, OPCODE);
            if(strcmp(intermediateLine[2], "RESB") == 0){
                fprintf(outputStream, "%s %s %s %s\n", ADDRESS, LABEL, OPCODE, OPERAND);
                if(textFlag != 0){
                    fflush(objectCodeStream);
                    textLengthPointer = fopen(objectCodeFileName, "r+");
                    fseek(textLengthPointer, textPosition, SEEK_SET);
                    fprintf(textLengthPointer, "%02X", textLength);
                    textPosition = 0;
                }
                textFlag = 0;
                textLength = 0;
            }
            if(strcmp(intermediateLine[2], "RESW") == 0){
                fprintf(outputStream, "%s %s %s %s\n", ADDRESS, LABEL, OPCODE, OPERAND);
                if(textFlag != 0){
                    fflush(objectCodeStream);
                    textLengthPointer = fopen(objectCodeFileName, "r+");
                    fseek(textLengthPointer, textPosition, SEEK_SET);
                    fprintf(textLengthPointer, "%02X", textLength);
                    textPosition = 0;
                }
                textFlag = 0;
                textLength = 0;
            }
            if(strcmp(intermediateLine[2], "BYTE") == 0){
                fprintf(outputStream, "%s %s %s %s ", ADDRESS, LABEL, OPCODE, OPERAND);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ^");
                }
                if((OPERAND[0] == 'C') && (OPERAND[1] == 39) && (OPERAND[strlen(OPERAND) - 1] == 39)){
                    for(int i=2;i<strlen(OPERAND)-1;i++){
                        fprintf(outputStream, "%02X", OPERAND[i]);
                        fprintf(objectCodeStream, "%02X", OPERAND[i]);
                        textLength++;
                    }
                    fprintf(outputStream, "\n");
                    if(textLength > 60){
                        if(textFlag != 0){
                            fflush(objectCodeStream);
                            textLengthPointer = fopen(objectCodeFileName, "r+");
                            fseek(textLengthPointer, textPosition, SEEK_SET);
                            fprintf(textLengthPointer, "%02X", textLength);
                            textPosition = 0;
                            textFlag = 0;
                            textLength = 0;
                        }
                    }
                }
                else if((OPERAND[0] == 'X') && (OPERAND[1] == 39) && (OPERAND[strlen(OPERAND) - 1] == 39)){
                    if(textFlag == 0){
                        textFlag = 1;
                        fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                        textPosition = ftell(objectCodeStream);
                        fprintf(objectCodeStream, "  ");
                    }
                    int tempHex = hex_to_int(OPERAND);
                    fprintf(outputStream, "%02X", tempHex);
                    fprintf(objectCodeStream, "%02X", tempHex);
                    textLength += 3;
                    fprintf(outputStream, "\n");
                    if(textLength > 60){
                        if(textFlag != 0){
                            fflush(objectCodeStream);
                            textLengthPointer = fopen(objectCodeFileName, "r+");
                            fseek(textLengthPointer, textPosition, SEEK_SET);
                            fprintf(textLengthPointer, "%02X", textLength);
                            textPosition = 0;
                            textFlag = 0;
                            textLength = 0;
                        }
                    }
                }
            }
            if(strcmp(intermediateLine[2], "WORD") == 0){
                fprintf(outputStream, "%s %s %s %s %06X\n", ADDRESS, LABEL, OPCODE, OPERAND, atoi(OPERAND));
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%06X", atoi(OPERAND));
                textLength += 3;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            if(instructionLength == 2){
                int firstReg = 0;
                int secondReg = 0;
                if(strlen(OPERAND) == 3){
                    if(strlen(OPERAND) == 3){
                        if(((firstReg = findRegValue(OPERAND[0])) != -1)){}
                        else{
                            printf("INVALID REGISTER : %c", OPERAND[0]);
                            return 1;
                        }
                        if(((secondReg = findRegValue(OPERAND[2])) != -1)){}
                        else{
                            printf("INVALID REGISTER : %c", OPERAND[2]);
                            return 1;
                        }
                    }
                }
                else if(strlen(OPERAND) == 1){
                    if(((firstReg = findRegValue(OPERAND[0])) != -1)){}
                    else{
                        printf("INVALID REGISTER : %c", OPERAND[0]);
                        return 1;
                    }
                }
                else{
                    printf("INVALID 2 ADDRESS FORMAT");
                    return 1;
                }
                fprintf(outputStream, "%s %s %s %s %s%d%d\n", ADDRESS, LABEL, OPCODE, OPERAND, findOpcode(opcodes, OPCODE), firstReg, secondReg);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ^");
                }
                fprintf(objectCodeStream, "^%s%d%d", findOpcode(opcodes, OPCODE), firstReg, secondReg);
                textLength += 2;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 3){
                char *currentOpcode = findOpcode(opcodes, OPCODE);
                currentOpcode[2] = '0';
                int firstThreeBits = (int)strtol(currentOpcode, NULL, 16) & 0xFFF;
                int lastThreeBits = 0;
                fprintf(outputStream, "%s %s %s %s", ADDRESS, LABEL, OPCODE, OPERAND);
                if(OPERAND[0] == '@'){
                    firstThreeBits = firstThreeBits | indirect_mask;
                }
                else if(OPERAND[0] == '#'){
                    firstThreeBits = firstThreeBits | immediate_mask;
                    char *token = strtok(OPERAND, "#");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(checkX(OPERAND)){
                    firstThreeBits = firstThreeBits | index_mask;
                }
                if(OPERAND[0] != '#' && OPERAND[0] != '@'){
                    firstThreeBits = firstThreeBits | program_counter_mask;
                    firstThreeBits = firstThreeBits | indirect_mask;
                    firstThreeBits = firstThreeBits | immediate_mask;
                    lastThreeBits = findSymbolAddress(symbols, OPERAND) - (int)strtol(ADDRESS, NULL, 16) - instructionLength;
                    if(lastThreeBits < 0){
                        //lastThreeBits *= -1;
                        lastThreeBits &= 0xFFF;
                    }
                }
                fprintf(outputStream, " %03X%03X\n", firstThreeBits, lastThreeBits);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%03X%03X", firstThreeBits, lastThreeBits);
                textLength += 3;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
            else if(instructionLength == 4){
                char *currentOpcode = findOpcode(opcodes, strtok(OPCODE, "+"));
                currentOpcode[2] = '0';
                int firstThreeBits = (int)strtol(currentOpcode, NULL, 16) & 0xFFF;
                int lastThreeBits = 0;
                firstThreeBits = firstThreeBits | extended_mask;
                fprintf(outputStream, "%s %s %s", ADDRESS, OPCODE, OPERAND);
                if(OPERAND[0] == '@'){
                    firstThreeBits = firstThreeBits | indirect_mask;
                    char *token = strtok(OPERAND, "@");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(OPERAND[0] == '#'){
                    firstThreeBits = firstThreeBits | immediate_mask;
                    char *token = strtok(OPERAND, "#");
                    lastThreeBits = atoi(token);
                    if(checkNumber(token)){
                        lastThreeBits = atoi(token);
                    }
                    else{
                        lastThreeBits = findSymbolAddress(symbols, token);
                    }
                }
                else if(checkX(OPERAND)){
                    firstThreeBits = firstThreeBits | index_mask;
                }
                if(OPERAND[0] != '#' && OPERAND[0] != '@'){
                    firstThreeBits = firstThreeBits | program_counter_mask;
                    firstThreeBits = firstThreeBits | indirect_mask;
                    firstThreeBits = firstThreeBits | immediate_mask;
                    lastThreeBits = findSymbolAddress(symbols, OPERAND);
                }
                fprintf(outputStream, " %03X%05X\n", firstThreeBits, lastThreeBits);
                if(textFlag == 0){
                    textFlag = 1;
                    fprintf(objectCodeStream, "\nT^%s^",ADDRESS);
                    textPosition = ftell(objectCodeStream);
                    fprintf(objectCodeStream, "  ");
                }
                fprintf(objectCodeStream, "^%03X%05X", firstThreeBits, lastThreeBits);
                textLength += 4;
                if(textLength > 60){
                    if(textFlag != 0){
                        fflush(objectCodeStream);
                        textLengthPointer = fopen(objectCodeFileName, "r+");
                        fseek(textLengthPointer, textPosition, SEEK_SET);
                        fprintf(textLengthPointer, "%02X", textLength);
                        textPosition = 0;
                        textFlag = 0;
                        textLength = 0;
                    }
                }
            }
        }
        temp = readNextLine(intermediateStream);
        numOps = splitCodeLine(temp, intermediateLine, delimeter, 4);
    }
    if(textFlag != 0){
        fflush(objectCodeStream);
        textLengthPointer = fopen(objectCodeFileName, "r+");
        fseek(textLengthPointer, textPosition, SEEK_SET);
        fprintf(textLengthPointer, "%02X", textLength);
        textPosition = 0;
    }
    if(numOps == 3){
        fprintf(outputStream, "%s %s %s\n", ADDRESS, OPCODE, OPERAND);
    }
    else if(numOps == 4){
        fprintf(outputStream, "%s %s %s %s\n", ADDRESS, LABEL, OPCODE, OPERAND);
    }
    fprintf(objectCodeStream, "\nE^%s", STARTINGADDRESS);
    return 1;
}

int pass1Loader(){
    char PROGADDR[6], *temp, temp1[4][20];
    int CSADDR, CSLTH = 0;
    printf("Enter the program address : \n");
    scanf("%s", PROGADDR);
    CSADDR = (int)strtol(PROGADDR, NULL, 16);
    printf("CSADDR : %X\n", CSADDR);

    FILE *obj1 = fopen("obj1", "r");
    if(obj1 == NULL){
        perror("Error opening object 1 file");
    }
    FILE *ESTAB = fopen("ESTAB", "w");
    if(ESTAB == NULL){
        perror("Error opening ESTAB file");
    }
    temp = readNextLine(obj1);
    while(temp != NULL){
        if(temp[0] == 'H'){
            CSADDR += CSLTH;
            splitCodeLine(temp, temp1, "^", 4);
            CSLTH = (int)strtol(temp1[3], NULL, 16);
        }
        else if(temp[0] == 'D'){
            int count = countDelimeters(temp, '^');
            char temp5[count + 1][20];
            splitCodeLine(temp, temp5, "^", count+1);
            for(int i=1;i<count;i+=2){
                fprintf(ESTAB, "%s %X\n", temp5[i], CSADDR + (int)strtol(temp5[i+1], NULL, 16));
            }
        }
        temp = readNextLine(obj1);
    }
    return 0;
}

char* readNextLine(FILE* stream){
    char *temp = malloc(100);
    if(fgets(temp, 100, stream) != NULL){
        return temp;
    }
    return NULL;
}

int splitCodeLine(char* str, char codeLine[][20], const char* delimeter, int size){
    char* token = strtok(str, delimeter);
    int i = 0;
    for(int i=0;i<size;i++){
        codeLine[i][0] = '\0';
    }
    while((token != NULL) && (i != size)){
        strcpy(codeLine[i], token);
        token = strtok(NULL, delimeter);
        i++;
    }
    int lengthTemp;
    for(int i=0;i<size;i++){
        lengthTemp = strlen(codeLine[i]);
        if(codeLine[i][lengthTemp-1] == '\n'){
            codeLine[i][lengthTemp-1] = '\0';
        }
    }
    if(codeLine[0][0] == '\0'){
        return 0;
    }
    return i;
}

SYMTAB* createSymtab(char symbol[], int locctr){
    SYMTAB* newNode = (SYMTAB*)malloc(sizeof(SYMTAB));
    newNode->locctr = locctr;
    strcpy(newNode->symbol, symbol);
    newNode->next = newNode;
    return newNode;
}

void insertSymbol(SYMTAB **head, char symbol[], int locctr){
    SYMTAB* newNode = createSymtab(symbol, locctr);
    if(*head == NULL){
        *head = newNode;
        newNode->next = *head;
    }
    else{
        SYMTAB* temp = *head;
        while(temp->next != *head){
            temp = temp->next;
        }
        temp->next = newNode;
        newNode->next = *head;
    }
}

bool findSymbol(SYMTAB* head, char symbol[]){
    if(head != NULL){
        SYMTAB* temp = head;
        do{
            if(strcmp(temp->symbol, symbol) == 0){
                return true;
            }
            temp = temp->next;
        }while(temp!=head);
    }
    return false;
}

int findSymbolAddress(SYMTAB* head, char symbol[]){
    if(head != NULL){
        SYMTAB* temp = head;
        do{
            if(strcmp(temp->symbol, symbol) == 0){
                return temp->locctr;
            }
            temp = temp->next;
        }while(temp!=head);
    }
    return -1;
}

bool checkComment(char codeLine[]){
    if(codeLine == NULL){
        return false;
    }
    else if(strlen(codeLine) < 2){
        return false;
    }
    return (codeLine[0] == '/' && codeLine[1] == '/');
}
OPTAB* createOpcode(char mnemonic[], int instructionLength, char opcode[]){
    OPTAB* newNode = (OPTAB*)malloc(sizeof(OPTAB));
    newNode->instructionLength = instructionLength;
    strcpy(newNode->mnemonic, mnemonic);
    strcpy(newNode->opcode, opcode);
    newNode->next = newNode;
    return newNode;
}

void insertOpcode(OPTAB **head, char mnemonic[], int instructionLength, char opcode[]){
    OPTAB* newNode = createOpcode(mnemonic, instructionLength, opcode);
    if(*head == NULL){
        *head = newNode;
        newNode->next = *head;
    }
    else{
        OPTAB* temp = *head;
        while(temp->next != *head){
            temp = temp->next;
        }
        temp->next = newNode;
        newNode->next = *head;
    }
}

int findInstructionLength(OPTAB* head, char mnemonic[]){
    if(head != NULL){
        if(mnemonic[0] == '+'){
            char *token = strtok(mnemonic, "+");
            if((token != NULL) && findInstructionLength(head, token) == 3){
                return 4;
            }
            else{
                return -1;
            }
        }
        OPTAB* temp = head;
        do{
            if(strcmp(temp->mnemonic, mnemonic) == 0){
                return temp->instructionLength;
            }
            temp = temp->next;
        }while(temp!=head);
    }
    return -1;
}

char* findOpcode(OPTAB* head, char mnemonic[]){
    if(head != NULL){
        OPTAB* temp = head;
        do{
            if(strcmp(temp->mnemonic, mnemonic) == 0){
                return temp->opcode;
            }
            temp = temp->next;
        }while(temp!=head);
    }
    return "";
}

void readOpcodes(OPTAB** head, FILE* opcodeStream){
    char* temp = readNextLine(opcodeStream);
    char codeLine[3][20];
    while(temp != NULL){
        splitCodeLine(temp, codeLine, " ", 3);
        insertOpcode(head, codeLine[0], atoi(codeLine[1]), codeLine[2]);
        temp = readNextLine(opcodeStream);
    }
}

void readSymtab(SYMTAB** head, FILE* symtabStream){
    char *temp = readNextLine(symtabStream);
    char symtabLine[2][20];
    while(temp != NULL){
        splitCodeLine(temp, symtabLine, " ", 2);
        insertSymbol(head, symtabLine[0], (int)strtol(symtabLine[1], NULL, 16));
        temp = readNextLine(symtabStream);
    }
}

void printHelp(char argv[]){
    printf("Usage : %s INPUTFILENAME OUTPUTFILENAME [-optionType option]...\n", argv);
    printf("Options:\n");
    printf("  -o, --opcode opcodeFile\t: Specifies the file for reading opcodes from.\n");
    printf("  -s, --symtab symtabFile\t: Specifies the file to write symbol table to.\n");
    printf("  -d, --delimeter delimeter\t: Prints this help message.\n");
    printf("  -h, --help help\t\t: Prints this help message.\n");
}

void printHelp2(char argv[]){
    printf("Usage : %s INTERMEDIATEFILE OPCODEFILENAME SYMTABFILENAME [-optionType option]...\n", argv);
    printf("Options:\n");
    printf("  -i, --intermediate intermediateFile\t: Specifies the intermediate file.\n");
    printf("  -0, --object objectFile\t\t: Specifies the object code file.\n");
    printf("  -d, --delimeter delimeter\t\t: Prints this help message.\n");
    printf("  -h, --help help\t\t\t: Prints this help message.\n");
}

int writeSymtabToFile(FILE* symtabStream, SYMTAB* head){
    if(head != NULL){
        SYMTAB* temp = head;
        do{
            fprintf(symtabStream, "%s %X\n", temp->symbol, temp->locctr);
            temp = temp->next;
        }while(temp!= head);
    }
    return -1;
}

bool checkHex(char str[]){
    for(int i=0;i<strlen(str);i++){
        if(!(((str[i] > 48) && (str[i] < 57)) || ((str[i] > 64) && (str[i] < 71)))){
            return false;
        }
    }
    return true;
}

char* tobinary(int n){
    char *b = malloc(16);
    b[0] = '0';
    int i=15;
    while(i>=0){
        int a = n & 1;
        b[i--] = (char)(a+'0');
        n >>= 1;
    }
    return b;
}

bool checkX(char str[]){
    char *token1 = strtok(str, ",");
    char *token2 = strtok(NULL, ",");
    if((token1 != NULL) && (token2 != NULL)){
        if((strcmp(token1, "X") == 0) || (strcmp(token2, "X") == 0)){
            return true;
        }
        return false;
    }
    return false;
}

bool checkNumber(char str[]){
    for(int i=0;i<strlen(str);i++){
        if(str[i] < '0' || str[i] > '9'){
            return false;
        }
    }
    return true;
}

int findRegValue(char temp){
    if(temp == 'A'){
        return 0;
    }
    if(temp == 'X'){
        return 1;
    }
    if(temp == 'B'){
        return 3;
    }
    if(temp == 'S'){
        return 4;
    }
    if(temp == 'T'){
        return 5;
    }
    if(temp == 'F'){
        return 6;
    }
    return -1;
}

int countDelimeters(char *str, char delimeter){
    int count = 0;
    for(int i=0;i<strlen(str);i++){
        if(str[i] == delimeter){
            count++;
        }
    }
    return count;
}

int hex_to_int(const char *input_string) {
    if (strncmp(input_string, "X'", 2) == 0 && input_string[strlen(input_string) - 1] == '\'') {
        char hex_part[100];
        strncpy(hex_part, input_string + 2, strlen(input_string) - 3);
        hex_part[strlen(input_string) - 3] = '\0';
        return (int)strtol(hex_part, NULL, 16);
    } else {
        fprintf(stderr, "Invalid format.\n");
        return -1;
    }
}
