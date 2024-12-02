#include"sic.h"

int main(int argc, char *argv[]){
    int numOfCompulsoryArguments = 3;
    if(argc < numOfCompulsoryArguments + 1){
        printHelp2(argv[0]);
        return 1;
    }
    char *outputFileName = "intermediate";
    char *symtabFileName = argv[3];
    char *opcodeFileName = argv[2];
    char *intermediateFileName = argv[1];
    char *objectCodeFileName = "objectCode";
    char *delimeter = " ";
    if(argc > numOfCompulsoryArguments + 1){
        for(int i=numOfCompulsoryArguments + 1;i<argc;i+=2){
            if((argc - numOfCompulsoryArguments - 1)%2 == 1){
                printHelp2(argv[0]);
                return 1;
            }
            else{
                if((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--intermediate") == 0)){
                    outputFileName = argv[i+1];
                }
                else if((strcmp(argv[i], "-O") == 0) || (strcmp(argv[i], "--object") == 0)){
                    objectCodeFileName= argv[i+1];
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
                    printHelp2(argv[0]);
                    return 1;
                }
                else{
                    printHelp2(argv[0]);
                    return 1;
                }
            }
        }
    }
    pass2(intermediateFileName, symtabFileName, opcodeFileName, outputFileName, objectCodeFileName, delimeter);
	return 0;
}
