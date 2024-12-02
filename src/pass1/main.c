#include "sic.h"

int main(int argc, char *argv[]){
    int numOfCompulsoryArguments = 2;
    if(argc < numOfCompulsoryArguments + 1){
        printHelp(argv[0]);
        return 1;
    }
    char *opcodeFileName = "opcode";
    char *symtabFileName = "symtab";
    char *objectFileName = "objectCode";
    char *delimeter = " ";
    char *codeFileName = argv[1];
    char *outputFileName = argv[2];
    if(argc > numOfCompulsoryArguments + 1){
        for(int i=numOfCompulsoryArguments + 1;i<argc;i+=2){
            if((argc - numOfCompulsoryArguments - 1)%2 == 1){
                printHelp(argv[0]);
                return 1;
            }
            else{
                if((strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "--opcode") == 0)){
                    opcodeFileName = argv[i+1];
                }
                else if((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--symtab") == 0)){
                    symtabFileName = argv[i+1];
                }
                else if((strcmp(argv[i], "-O") == 0) || (strcmp(argv[i], "--object") == 0)){
                    objectFileName = argv[i+1];
                }
                else if((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--delimeter") == 0)){
                    if(strcmp(argv[i+1],"space") == 0){
                        delimeter = " ";
                    }
                    else if(strcmp(argv[i+1],"tab") == 0){
                        delimeter = "\t";
                    }
                }
                else if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "help") == 0)){
                    printHelp(argv[0]);
                    return 1;
                }
                else{
                    printHelp(argv[0]);
                    return 1;
                }
            }
        }
    }
    pass1(codeFileName, outputFileName, opcodeFileName, symtabFileName, objectFileName, delimeter);
    return 0;
}
