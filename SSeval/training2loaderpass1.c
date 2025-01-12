#include<stdio.h>
#include<string.h>

struct externalSymbolTable{
    char controlSectionName[20];
    char symbolName[20];
    int address;
    int length;
}symbols[20];
int symbolCount = 0;
int main(){
    FILE *inputFile, *outputFile;
    char controlSectionName[20], recordType[20];
    int address, length = 0, controlSectionAddress, programAddress;
    inputFile = fopen("INPUT", "r");
    outputFile = fopen("ESTAB", "w");
    printf("Enter the starting address : ");
    scanf("%X", &programAddress);
    controlSectionAddress = programAddress;
    fscanf(inputFile, "%s", recordType);
    while(strcmp(recordType, "END") != 0){
        if(strcmp(recordType, "H") == 0){
            fscanf(inputFile, "%s %X %X", symbols[symbolCount].controlSectionName, &symbols[symbolCount].address, &symbols[symbolCount].length);
            length = symbols[symbolCount].length;
            symbols[symbolCount].address = controlSectionAddress;
            fprintf(outputFile, "%s %X %X\n", symbols[symbolCount].controlSectionName, symbols[symbolCount].address, symbols[symbolCount].length);
            symbolCount++;
        } else if(strcmp(recordType, "D") == 0){
            fscanf(inputFile, "%s", recordType);
            while(strcmp(recordType, "R") != 0){
                strcpy(symbols[symbolCount].symbolName, recordType);
                fscanf(inputFile, "%X", &symbols[symbolCount].address);
                symbols[symbolCount].address += controlSectionAddress;
                fprintf(outputFile, "%s %X\n", symbols[symbolCount].symbolName, symbols[symbolCount].address);
                symbolCount++;
                fscanf(inputFile, "%s", recordType);
            }
            controlSectionAddress+=length;
        } else if(strcmp(recordType, "T") == 0){
            while(strcmp(recordType, "E") != 0){
                fscanf(inputFile, "%s", recordType);
            }
        }
        fscanf(inputFile, "%s", recordType);
    }
}
