/*
* Struct definitions
*/
typedef struct CodeNode {
    char key;
    int freq;
    struct CodeNode *left;
    struct CodeNode *right;
} CodeNode;

typedef struct CodeList {
    CodeNode *root;
    int size;
} CodeList;

/*
* Function declarations
*/
char * readFromFile(char *);
void writeToFile(char *, char *, int);
void compressAndWriteToFile(CodeList *, int *, char *, char *);
void decompressAndWriteToFile(char *, char *);
int numberBits(int);
int numberBytes(int);
CodeList * duplicateCodeList(CodeList *);
void freeCodeList(CodeList *);
void printString(char *text);
int findStringSize(char *);
