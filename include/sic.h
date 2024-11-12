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
    char opcode[20];
    struct opcodeTable *next;
}OPTAB;

char* readNextLine(FILE* stream);
int pass1(const char* codeFileName, const char* outputFileName, const char* opcodeFileName, const char* symtabFileName, const char* delimeter);
int pass2(const char* intermediateFileName, const char* symtabFileName, const char* opcodeFileName, const char* outputFileName, const char* objectCodeFileName, const char* delimeter);
int splitCodeLine(char* str, char codeLine[][20], const char *delimeter, int size);
SYMTAB* createSymtab(char symbol[], int locctr);
void insertSymbol(SYMTAB **head, char symbol[], int locctr);
bool findSymbol(SYMTAB* head, char symbol[]);
OPTAB* createOpcode(char mnemonic[], int instructionLength, char opcode[]);
void insertOpcode(OPTAB **head, char mnemonic[], int instructionLength, char opcode[]);
int findInstructionLength(OPTAB* head, char mnemonic[]);
void readOpcodes(OPTAB** head, FILE* opcodeStream);
void readSymtab(SYMTAB** head, FILE* symtabStream);
bool checkComment(char codeLine[]);
void printHelp(char argv[]);
int writeSymtabToFile(FILE* symtabStream, SYMTAB* head);
bool checkNumber(char str[]);
char* tobinary(int n);

#endif
