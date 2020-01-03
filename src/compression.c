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
int compressDictionary(CodeList *, int *, char *, int);
int compressDictionaryCode(char *, int, int, int);
int compressText(int *, char *, char *, int);
int compressTextCode(char *, int, int, int);
int decompressDictionary(unsigned char *, int, int *, int);
int decompressDictionaryCode(unsigned char *, int);
char * decompressText(unsigned char *, int, int *);
char findKeyFromCode(int *, int);
char appendBitsToByte(char *, int, int);


/*
* Function definitions
*/
char * readFromFile(char *filename) {
    FILE *fp = fopen(filename, "rb");
    int currentLength = 0;
    int maxLength = 100;
    char *fullText = malloc(sizeof(char) * maxLength);
    int currentValue = 0;

    if(fp == NULL) {
        printf("Error while opening file %s\n", filename);
    }

    while((currentValue = fgetc(fp)) != EOF) {
        *(fullText + currentLength) = (char) currentValue;
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

void writeToFile(char *filename, char *text, int size) {
    FILE *file = fopen(filename, "wb");

    int results = fwrite(text, sizeof(char), size, file);
    if (results == EOF) {
        printf("Failed to write");
    }
    fclose(file);
}

void compressAndWriteToFile(CodeList *codeList, int *charDict, char *text, char *filename) {
    int size = findCompressedSize(codeList, charDict, text);
    int index = 0;
    char compressedText[size];
    memset(compressedText, 0, sizeof(char) * size);
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

void decompressAndWriteToFile(char *compressedFilename, char *uncompressedFilename) {
    char *compressedText = readFromFile(compressedFilename);
    int dictionarySize = compressedText[0];
    int index = 1;
    int charDict[256];
    memset(charDict, 0, sizeof(int) * 256);
    index = decompressDictionary(compressedText, index, charDict, dictionarySize);
    char *uncompressedText = decompressText(compressedText, index, charDict);
    printf("\n%d\n", findStringSize(uncompressedText));
    writeToFile(uncompressedFilename, uncompressedText, findStringSize(uncompressedText));
}

int decompressDictionary(unsigned char *compressedText, int index, int *charDict, int dictionarySize) {
    for(int i = 0; i < dictionarySize; i++) {
        int key = compressedText[index];
        int numberBytes = compressedText[index + 1];
        
        charDict[key] = decompressDictionaryCode(compressedText + index + 2, numberBytes);
        index = index + 2 + numberBytes;
    }

    return index;
}

int decompressDictionaryCode(unsigned char *compressedText, int bytes) {
    int bitBytes = 8;
    int value = 0;

    for(int i = 0; i < bytes; i++) {
        int current = compressedText[i];
        value = (current << (bitBytes * i)) + value;
    }

    return value;
}

char * decompressText(unsigned char *compressedText, int index, int *charDict) {
    int currentLength = 0;
    int maxLength = 100;
    char *uncompressedText = malloc(sizeof(char) * maxLength);

    int currentBit = 7;
    int currentValue = -1;
    char foundValue = 0;

    while(currentValue != 0) {
        if(currentValue == -1) currentValue = 0;
        currentValue = (currentValue << 1) + ((compressedText[index] >> currentBit) & 1);

        if((foundValue = findKeyFromCode(charDict, currentValue)) != 0) {
            uncompressedText[currentLength] = foundValue;
            currentLength += 1;
            currentValue = -1;

            if(currentLength == maxLength) {
                maxLength *= 2;
                uncompressedText = realloc(uncompressedText, sizeof(char) * maxLength);
            }
        }

        if(currentBit == 0) {
            currentBit = 7;
            index += 1;
        }
        else {
            currentBit -= 1;
        }
    }

    *(uncompressedText + currentLength) = '\0';
    uncompressedText = realloc(uncompressedText, sizeof(char)*(currentLength + 1));
    return uncompressedText;
}

char findKeyFromCode(int *charDict, int code) {
    for(int i = 0; i < 256; i++) {
        if(charDict[i] == code) return (char) i;
    }

    return 0;
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

int findStringSize(char *text) {
    int i = 0;
    while(*(text + i) != '\0') i += 1;
    return i;
}

