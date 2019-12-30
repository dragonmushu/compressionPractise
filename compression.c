#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
* Struct definitions
*/
typedef struct Code {
    char key;
    int freq;
    struct Code *left;
    struct Code *right;
} Code;

typedef struct CodeList {
    Code *codes;
    int size;
} CodeList;

/*
* Function declarations
*/
char * readFromFile(char *);
void writeToFile(char *, char *);
void compressAndWriteToFile(CodeList *, int *, char *);
int findCompressedSize(CodeList *codeList, int *charDict, char *text, char *filename);
int findCompressedDictionarySize(CodeList *codeList, int *charDict);
int findCompressedTextSize(int *charDict, char *text);
int numberBits(int);
int numberBytes(int);

/*
* Function definitions
*/
char * readFromFile(char *filename) {
    FILE *fp = fopen(filename, "r");
    int currentLength = 0;
    int maxLength = 100;
    char *fullText = malloc(sizeof(char) * maxLength);

    if(fp == NULL) {
        printf("Error while opening file %s\n", filename);
    }

    while((*(fullText + currentLength) = fgetc(fp)) != EOF) { 
        currentLength += 1;
        if(currentLength == maxLength) {
            maxLength *= 2;
            fullText = realloc(fullText, sizeof(char) * maxLength);
        }
    }

    *(fullText + currentLength) = '\0';
    fullText = realloc(fullText, sizeof(char)*(currentLength + 1));
    fclose(fp);

    return fullText;
}

void writeToFile(char *filename, char *text) {
    FILE *file = fopen(filename, "w");

    int results = fputs(text, file);
    if (results == EOF) {
        printf("Failed to write");
    }
    fclose(file);
}

void compressAndWriteToFile(CodeList *codeList, int *charDict, char *text, char *filename) {
    int size = findCompressedSize(codeList, charDict, text);
    char compressedText[size];
    int index = compressDictionary(codeList, charDict, &compressedText, 0);
    index = compressText(charDict, text, &compressedText, index);
    writeToFile(filename, compressedText);
}

int findCompressedSize(CodeList *codeList, int *charDict, char *text) {
    return findCompressedDictionarySize(codeList, charDict) + findCompressedTextSize(charDict, text);
}

int findCompressedDictionarySize(CodeList *codeList, int *charDict) {
    int size = 1;
    for(int i = 0; i < codeList->size; i++) {
        int key = codeList->codes[i].key;
        int keyBytes = numberBytes(key);
        size = size + 2 + keyBytes;
    }
    return size;
}

int findCompressedTextSize(int *charDict, char *text) {
    int bits = 0;
    while(*text != '\0') {
        int key = *text;
        bits += numberBits(charDict[key]); 
        text += 1;
    }
    int size = bits / 8 + 1;
    return size;
}

int compressDictionary(CodeList *codeList, int *charDict, char *compressedText, int index) {
    int bitBytes = 8;

    compressedText[index++] = appendBitsToByte(compressedText + index, codeList->size, bitBytes); // add size of code list
    
    for(int i = 0; i < codeList->size; i++) {
        int key = codeList->code[i].key;
        int codeBytes = numberBytes(charDict[key]);
        int compressedValue = charDict[key];

        compressedText[index++] = appendBitsToByte(compressedText + index, key, bitBytes);
        compressedText[index++] = appendBitsToByte(compressedText + index, codeBytes, bitBytes);
        index = compressDictionaryCode(compressedText, index, compressedValue, codeBytes); 
    }
    
    return index;
}

int compressDictionaryCode(char *compressedText, int index, int value, int numberBytes) {
    int bitBytes = 8;

    for(int i = 0; i < numberBytes; i++) {
        int mask = 255;
        compressedText[index++] = appendBitsToByte(compressedText + index, value & mask, bitBytes);
        mask = mask << bitBytes;
    }

    return index;
}

int compressText(int *charDict, char *text, char *compressedText, int index) {
    int currentBit = 0;

    while(*text != '\0') {
        int key = *compressedText;
        int code = charDict[key];


        text += 1;
    }
}

int compressTextCode(char *compressedText, int index, int currentBit, int code) {
    int bitBytes = 8;
    int codeBitsLeft = numberBits(code);
    int numberBitsInByte = bitBytes - currentBit;

    while(codeBitsLeft >= numberBitsInByte) {
        appendBitsToByte(compressedText, )
    }


}

char appendBitsToByte(char *initial, int value, int shift) {
    return (*initial << shift) + value;
}

int numberBits(int value) {
    int count = 0;
    while(value != 0) {
        value = value >> 1;
        count += 1;
    }

    return count;
}

int numberBytes(int value) {
    int count = 0;
    while(value != 0) {
        value = value >> 8;
        count += 1;
    }

    return count;
}