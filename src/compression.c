#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compression.h"

/*
* Function declarations
*/
int findCompressedSize(CodeList *codeList, int *charDict, char *text);
int findCompressedDictionarySize(CodeList *codeList, int *charDict);
int findCompressedTextSize(int *charDict, char *text);
int compressFileSize(int, char *);
int compressDictionary(CodeList *, int *, char *, int);
int compressDictionaryCode(char *, int, int, int);
int compressText(int *, char *, char *, int);
int compressTextCode(char *, int, int, int);
char appendBitsToByte(char *, int, int);


/*
* Function definitions
*/
char * readFromFile(char *filename, int readBytes) {
    FILE *fp = fopen(filename, "r");
    int currentLength = 0;
    int maxLength = 2200;
    char *fullText = malloc(sizeof(char) * maxLength);
    int currentByte = 0;
    int currentValue = 0;

    if(fp == NULL) {
        printf("Error while opening file %s\n", filename);
    }

    while(currentByte < readBytes || readBytes == -1) {
        currentValue = fgetc(fp);
        if(readBytes == -1 && currentValue == EOF) {
            break; 
        }

        *(fullText + currentLength) = (char) currentValue;
        currentLength += 1;
        if(currentLength == maxLength) {
            maxLength *= 2;
            fullText = realloc(fullText, sizeof(char) * maxLength);
        }
        currentByte += 1;
        printf("%d\n", currentValue);
    }

    *(fullText + currentLength) = '\0';
    fullText = realloc(fullText, sizeof(char)*(currentLength + 1));
    fclose(fp);

    return fullText;
}

void writeToFile(char *filename, char *text, int size) {
    for(int i = 0; i < size; i++) {
        printf("%d\n", text[i]);
    }

    FILE *file = fopen(filename, "w");
    printf("%d", size);
    int results = fwrite(text, sizeof(char), size, file);
    if (results == EOF) {
        printf("Failed to write");
    }
    fclose(file);
}

void compressAndWriteToFile(CodeList *codeList, int *charDict, char *text, char *filename) {
    int size = findCompressedSize(codeList, charDict, text);
    int intSize = 4;
    size += intSize;
    char compressedText[size];
    memset(compressedText, 0, sizeof(char) * size);
    int index = compressFileSize(size, compressedText);
    index = compressDictionary(codeList, charDict, compressedText, index);
    index = compressText(charDict, text, compressedText, index);
    writeToFile(filename, compressedText, size);
}

int findCompressedSize(CodeList *codeList, int *charDict, char *text) {
    return findCompressedDictionarySize(codeList, charDict) + findCompressedTextSize(charDict, text);
}

int findCompressedDictionarySize(CodeList *codeList, int *charDict) {
    int size = 1;
    for(int i = 0; i < codeList->size; i++) {
        int key = codeList->root[i].key;
        int keyBytes = numberBytes(charDict[key]);
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

int compressFileSize(int size, char *compressedText) {
    int bitBytes = 8;
    int mask = 255;
    int intSize = 4;

    for(int i = 0; i < intSize; i++) {
        compressedText[i] = appendBitsToByte(compressedText + i, size & mask, bitBytes);
        size = size >> 8;
    }

    return intSize;
}

int compressDictionary(CodeList *codeList, int *charDict, char *compressedText, int index) {
    int bitBytes = 8;

    compressedText[index++] = appendBitsToByte(compressedText + index, codeList->size, bitBytes); // add size of code list
    
    for(int i = 0; i < codeList->size; i++) {
        int key = codeList->root[i].key;
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
    int mask = 255;
    
    for(int i = 0; i < numberBytes; i++) {
        compressedText[index++] = appendBitsToByte(compressedText + index, value & mask, bitBytes);
        value = value >> bitBytes;
    }

    return index;
}

int compressText(int *charDict, char *text, char *compressedText, int index) {
    int currentBit = 0;

    while(*text != '\0') {
        int key = *text;
        int code = charDict[key];
        index = compressTextCode(compressedText, index, currentBit, code);
        currentBit = (currentBit + numberBits(code)) % 8;
        text += 1;
    }

    return index;
}

int compressTextCode(char *compressedText, int index, int currentBit, int code) {
    int bitBytes = 8;
    int codeBitsLeft = numberBits(code);
    int numberBitsInByte = bitBytes - currentBit;
    

    while(codeBitsLeft >= numberBitsInByte) {
        int mask = (1 << (codeBitsLeft - numberBitsInByte)) - 1;
        compressedText[index++] = appendBitsToByte(compressedText + index, code >> (codeBitsLeft - numberBitsInByte), numberBitsInByte);
        code = code & mask;
        codeBitsLeft -= numberBitsInByte;
        numberBitsInByte = 8;
    }
    
    compressedText[index] = appendBitsToByte(compressedText + index, code, codeBitsLeft);
    return index;
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

CodeList * duplicateCodeList(CodeList *codes) {
    CodeList *copy = malloc(sizeof(CodeList));
    copy->root = malloc(sizeof(CodeNode) * codes->size);
    memcpy(copy->root, codes->root, codes->size * sizeof(CodeNode));
    copy->size = codes->size;
    return copy;
}

void freeCodeList(CodeList *codes) {
    free(codes->root);
    free(codes);
}

void printString(char *text) {
    while(*text != '\0') {
        printf("%c", *text);
        text += 1;
    }
}

void printCharInt(char *text) {
    while(*text != '\0') {
        printf("%d\n", *text);
        text += 1;
    }
}

