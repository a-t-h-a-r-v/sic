#ifndef SIC_H
#define SIC_H

#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

#define indirect_mask 32
#define immediate_mask 16
#define index_mask 8
#define base_mask 4
#define program_counter_mask 2
#define extended_mask 1

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

typedef struct objectCode{
    int address;
}OBJECT;

char* readNextLine(FILE* stream);
int pass1(const char* codeFileName, const char* outputFileName, const char* opcodeFileName, const char* symtabFileName, const char* objectCodeFileName, const char* delimeter);
int pass2(const char* intermediateFileName, const char* symtabFileName, const char* opcodeFileName, const char* outputFileName, const char* objectCodeFileName, const char* delimeter);
int pass1Loader();
int splitCodeLine(char* str, char codeLine[][20], const char *delimeter, int size);
SYMTAB* createSymtab(char symbol[], int locctr);
void insertSymbol(SYMTAB **head, char symbol[], int locctr);
bool findSymbol(SYMTAB* head, char symbol[]);
int findSymbolAddress(SYMTAB* head, char symbol[]);
OPTAB* createOpcode(char mnemonic[], int instructionLength, char opcode[]);
void insertOpcode(OPTAB **head, char mnemonic[], int instructionLength, char opcode[]);
int findInstructionLength(OPTAB* head, char mnemonic[]);
char* findOpcode(OPTAB* head, char mnemonic[]);
void readOpcodes(OPTAB** head, FILE* opcodeStream);
void readSymtab(SYMTAB** head, FILE* symtabStream);
bool checkComment(char codeLine[]);
void printHelp(char argv[]);
void printHelp2(char argv[]);
int writeSymtabToFile(FILE* symtabStream, SYMTAB* head);
bool checkNumber(char str[]);
bool checkHex(char str[]);
bool checkX(char str[]);
char* tobinary(int n);
int findRegValue(char temp);
int countDelimeters(char *str, char delimeter);
int hex_to_int(const char *input_string);

#endif
