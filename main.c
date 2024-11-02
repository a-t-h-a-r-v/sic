#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

typedef struct symbolTable{
    char symbol[20];
    int locctr;
    struct symbolTable *next;
}SYMTAB;

char* readNextLine(FILE* stream);
int pass1(const char* codeFileName, const char* outputFileName, const char* opcodeFileName);
int splitCodeLine(char* str, char codeLine[3][20]);
SYMTAB* createSymtab(char symbol[], int locctr);
void insertSymbol(SYMTAB **head, char symbol[], int locctr);
bool findSymbol(SYMTAB* head, char symbol[]);
bool checkComment(char codeLine[]);

int main(int argc, char *argv[]){
    pass1("temp1", "temp2", "temp3");
    return 0;
}

int pass1(const char* codeFileName, const char* outputFileName, const char* opcodeFileName){
    FILE* codeStream = fopen(codeFileName, "r");
    if(codeStream == NULL){
        perror("Error opening input file");
        return 1;
    }
    FILE* outputStream = fopen(outputFileName, "w");
    if(outputStream == NULL){
        perror("Error opening input file");
        return 1;
    }
    FILE* opcodeStream = fopen(opcodeFileName, "r");
    if(opcodeStream == NULL){
        perror("Error opening input file");
        return 1;
    }

    char codeLine[3][20];
    char *OPCODE;
    char *OPERAND;
    char *LABEL;
    int LOCCTR, STARTINGADDRESS;

    char *temp = readNextLine(codeStream);
    int numOps = splitCodeLine(temp, codeLine);
    if(numOps == 2){
        OPCODE = codeLine[0];
        OPERAND = codeLine[1];
        if(strcmp(OPCODE, "START") == 0){
            STARTINGADDRESS = atoi(OPERAND);
            LOCCTR = STARTINGADDRESS;
        }
    }

    else{
        STARTINGADDRESS = 0;
        LOCCTR = 0;
    }
    temp = readNextLine(codeStream);
    while(checkComment(temp)){
        temp = readNextLine(codeStream);
    }
    printf("%s", temp);
    numOps = splitCodeLine(temp, codeLine);
    SYMTAB* symbols = NULL;

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

    if(LABEL != NULL){
        if(findSymbol(symbols, LABEL)){
            printf("DUPLICATE SYMBOL FOUND : %s", LABEL);
            return 1;
        }
        else{
            insertSymbol(&symbols, LABEL, LOCCTR);
        }
    }

    fclose(codeStream);
    fclose(outputStream);
    fclose(opcodeStream);
    return 1;
}

char* readNextLine(FILE* stream){
    char *temp = malloc(100);
    if(fgets(temp, 100, stream) != NULL){
        return temp;
    }
    return NULL;
}

int splitCodeLine(char* str, char codeLine[3][20]){
    char* token = strtok(str, " ");
    int i = 0;
    codeLine[0][0] = '\0';
    codeLine[1][0] = '\0';
    codeLine[2][0] = '\0';
    while((token != NULL) && (i != 3)){
        strcpy(codeLine[i], token);
        token = strtok(NULL, " ");
        i++;
    }
    int lengthTemp = strlen(codeLine[2]);
    if(codeLine[2][lengthTemp-1] == '\n'){
        codeLine[2][lengthTemp-1] = '\0';
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

bool checkComment(char codeLine[]){
    if(strlen(codeLine) < 2){
        return false;
    }
    else if(codeLine[0] == '/' && codeLine[1] == '/'){
        return true;
    }
    else{
        return false;
    }
}
