#ifndef SIC_H
#define SIC_H

#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

typedef struct symbolTable{
    char symbol[20];
    int locctr;
    struct symbolTable *next;
}SYMTAB;

typedef struct opcodeTable{
    char mnemonic[20];
    int instructionLength;
    struct opcodeTable *next;
}OPTAB;

char* readNextLine(FILE* stream);
int pass1(const char* codeFileName, const char* outputFileName, const char* opcodeFileName);
int splitCodeLine(char* str, char codeLine[3][20]);
SYMTAB* createSymtab(char symbol[], int locctr);
void insertSymbol(SYMTAB **head, char symbol[], int locctr);
bool findSymbol(SYMTAB* head, char symbol[]);
OPTAB* createOpcode(char mnemonic[], int instructionLength);
void insertOpcode(OPTAB **head, char mnemonic[], int instructionLength);
int findInstructionLength(OPTAB* head, char mnemonic[]);
void readOpcodes(OPTAB** head, FILE* opcodeStream);
bool checkComment(char codeLine[]);
void printHelp(char argv[]);

#endif
