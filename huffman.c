#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Code {
    char key;
    int freq;
    struct Code *left;
    struct Code *right;
} Code;

typedef struct HuffmanCodes {
    Code *code;
    int size;
} HuffmanCodes;


/*
* Function Definitions
*/
char * parseFileToString(char *);
HuffmanCodes * textToCharCodes(char *);
void charFrequency(char *, int *);
int obtainValidDictLength(int *);
Code * frequencyToCodes(int *, int);
HuffmanCodes * buildHuffmanHeap(HuffmanCodes *);
void forwardHeapify(HuffmanCodes *, int); 
void backwardHeapify(HuffmanCodes *, int);
int findMinFreqIndex(HuffmanCodes *, int, int, int);
void swapCodes(Code *, int, int);
Code * buildHuffmanTree(HuffmanCodes *);
Code heapPop(HuffmanCodes *);
void heapPush(HuffmanCodes *, Code);
int * treeToCharDict(Code *);
void codifyTree(Code *, int *, int);
HuffmanCodes * duplicateHuffmanDictionary(HuffmanCodes *);
void freeHuffmanTree(Code *);
void freeHuffmanDictionary(HuffmanCodes *);
int dictionaryToByteArray(HuffmanCodes *, int *, char *);
int numberBits(int);
int numberBytes(int);
int appendValueToCompressedText(char *, int, int, int);
int appendIntToByteArray(char *, int, int);
void appendBitsToByte(char *, int, int, int);
void compressAllText(char *, char *, int *, int);
int maskLeastSigDigits(int, int, int);
void writeToFile(char *, char *);

void huffmanEncode(char *filename) {
    char *text = parseFileToString(filename);
    HuffmanCodes *codeList = textToCharCodes(text);
    HuffmanCodes *codeHeap = duplicateHuffmanDictionary(codeList);

    codeHeap = buildHuffmanHeap(codeHeap);
    Code *root = buildHuffmanTree(codeHeap);
    int * charDict = treeToCharDict(root);

    int bytesUncompressed = 0;
    int bytesCompressed = 0;
    for(int i = 0; i < codeList->size; i++) {
        int freq = codeList->code[i].freq;
        int key = codeList->code[i].key;
        int compressedBits = numberBits(charDict[key]);
        bytesUncompressed += (freq * 8);
        bytesCompressed += (freq * compressedBits);
        bytesCompressed += (8*2  + compressedBits + compressedBits % 8);
    }
    bytesCompressed += 8;
    printf("%d\n", bytesUncompressed);
    printf("%d\n", bytesCompressed);


    freeHuffmanDictionary(codeHeap);
    freeHuffmanDictionary(codeList);
    freeHuffmanTree(root);

}

void huffmanDecode(char *filename) {
}

char * parseFileToString(char *filename) {
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

HuffmanCodes * textToCharCodes(char *text) {
    int charDict[256] = {0};
    charFrequency(text, charDict);
    int length = obtainValidDictLength(charDict);
    HuffmanCodes *codes = malloc(sizeof(HuffmanCodes));
    codes->size = length;
    codes->code = frequencyToCodes(charDict, length);

    return codes;
}

void charFrequency(char *text, int *charDict) {
    while(*text != '\0') {
        int index = *text;
        charDict[index] += 1;
        text += 1;
    }
}

int obtainValidDictLength(int *dict) {
    int count = 0;
    for (int i = 0; i < 256; i++) {
        if(dict[i] != 0) count += 1;
    }

    return count;
}

Code * frequencyToCodes(int *charDict, int length) {
    Code *codes = malloc(sizeof(HuffmanCodes));
    codes = (Code *) malloc(sizeof(Code) * length);
    int currentIndex = 0;

    for(int i = 0; i < 256; i++) {
        if(charDict[i] != 0) {
            codes[currentIndex].key = i;
            codes[currentIndex].freq = charDict[i];
            codes[currentIndex].left = NULL;
            codes[currentIndex].right = NULL;
            currentIndex += 1;
        }
    }

    return codes;
}

HuffmanCodes * buildHuffmanHeap(HuffmanCodes *codes) {
    int currentIndex = (codes->size - 1) / 2;
    while(currentIndex >= 0) {
        forwardHeapify(codes, currentIndex);
        currentIndex -= 1;
    }

    return codes;
}

void forwardHeapify(HuffmanCodes * codes, int current) {
    int left = current * 2 + 1;
    int right = current * 2 + 2;
    int minFreqIndex = findMinFreqIndex(codes, current, left, right);
    
    if(minFreqIndex == left) {
        swapCodes(codes->code, left, current);
        forwardHeapify(codes, left);
    }
    if(minFreqIndex == right) {
        swapCodes(codes->code, right, current);
        forwardHeapify(codes, right);
    }
}

void backwardHeapify(HuffmanCodes * codes, int current) {
    if(current != 0) {
        int parent = (current - 1) / 2;
        if(codes->code[current].freq < codes->code[parent].freq) {
            swapCodes(codes->code, current, parent);
            backwardHeapify(codes, parent);
        }
    }
}

int findMinFreqIndex(HuffmanCodes * codes, int current, int left, int right) {
    int leftFreq = codes->code[current].freq;
    int rightFreq = codes->code[current].freq;
    int currentFreq = codes->code[current].freq;

    if(left < codes->size) leftFreq = codes->code[left].freq;
    if(right < codes->size) rightFreq = codes->code[right].freq;

    if(leftFreq <= rightFreq && leftFreq < currentFreq) return left;
    if(rightFreq <= leftFreq && rightFreq < currentFreq) return right;
    return current;
} 

void swapCodes(Code *codes, int firstIndex, int secondIndex) {
    Code temp = codes[firstIndex];
    codes[firstIndex] = codes[secondIndex];
    codes[secondIndex] = temp;
}

Code * buildHuffmanTree(HuffmanCodes * codes) {
    while (codes->size > 1) {
        Code *min1 = malloc(sizeof(Code));
        Code *min2 = malloc(sizeof(Code));
        *min1 = heapPop(codes);
        *min2 = heapPop(codes);

        Code root;
        root.left = min2;
        root.right = min1;
        root.freq = min1->freq + min2->freq;
        root.key = -1;
        heapPush(codes, root);
    }

    Code *root = malloc(sizeof(Code));
    *root = heapPop(codes);
    return root;
}

Code heapPop(HuffmanCodes * codes) {
    Code top = codes->code[0];
    codes->size -= 1;
    if (codes->size != 0) {
        swapCodes(codes->code, 0, codes->size);
        forwardHeapify(codes, 0);
    }
    
    return top;
}

void heapPush(HuffmanCodes * codes, Code code) {
    codes->code[codes->size] = code;
    backwardHeapify(codes, codes->size);
    codes->size += 1;
}

int * treeToCharDict(Code *root) {
    int * charDict = malloc(sizeof(int) * 256);
    codifyTree(root, charDict, 1);

    return charDict;
}

void codifyTree(Code *root, int *codes, int code) {
    if(root != NULL) {
        codifyTree(root->left, codes, (code << 1) + 1);
        if(root->key != -1) {
            int index = root->key;
            codes[index] = code;
        }
        codifyTree(root->right, codes, code << 1);
    }
}

void freeHuffmanTree(Code *root) {
    if(root != NULL) {
        freeHuffmanTree(root->left);
        freeHuffmanTree(root->right);
        free(root);
    }
}

void freeHuffmanDictionary(HuffmanCodes * codes) {
    free(codes->code);
    free(codes);
}

HuffmanCodes * duplicateHuffmanDictionary(HuffmanCodes * codes) {
    HuffmanCodes * copy = malloc(sizeof(HuffmanCodes));
    copy->code = malloc(codes->size * sizeof(Code));
    memcpy(copy->code, codes->code, codes->size * sizeof(Code));
    copy->size = codes->size;
    return copy;
}

void createCompressedFile(char *text, HuffmanCodes *codeList, int *charDict, int size) {

    char compressedText[size];
    int index = dictionaryToByteArray(codeList, charDict, &compressedText);
    compressedText[index++] = 0;
    compressAllText(&compressedText, text, charDict, index);
    writeToFile("testC.txt", &compressedText);

}

int dictionaryToByteArray(HuffmanCodes *codeList, int *charDict, char *compressed) {
    int bitBytes = 8; 
    
    int index = 0;
    for(int i = 0; i < codeList->size; i++) {
        int key = codeList->code[i].key;
        int codeBytes = numberBits(charDict[key]);

        appendBitsToByte(compressed, index, key, bitBytes);
        appendBitsToByte(compressed, index + 1, codeBytes, bitBytes);
        appendIntToByteArray(compressed, index + 2, charDict[key]); 
        index += (codeBytes + 2);
    }
    
    return index;
}


void compressAllText(char *compressed, char *text, int *charDict, int index) {
    int textIndex = 0;
    int currentBit = 0;

    while(text[textIndex] != '\0') {
        index = appendValueToCompressedText(compressed, index, currentBit, text[textIndex]);
        currentBit = (currentBit + text[textIndex]) % 8;
    }
}


int appendValueToCompressedText(char *compressedText, int index, int currentBit, int value) {
    int byteBits = 8;
    int compressedBitsLeft = byteBits - currentBit; 
    int valueBitsLeft = numberBits(value); 

    if(valueBitsLeft < compressedBitsLeft) {
        appendBitsToByte(compressedText, index, value, valueBitsLeft);
        return index;
    }

    while(valueBitsLeft >= byteBits) {
        appendBitsToByte(compressedText, index, value >> (valueBitsLeft - compressedBitsLeft), compressedBitsLeft);
        value = maskLeastSigDigits(value, valueBitsLeft);
        valueBitsLeft -= compressedBitsLeft;
        index += 1;
        compressedBitsLeft = byteBits;
    }

    appendBitsToByte(compressedText, index, value, valueBitsLeft);
    return index;
}

void appendIntToByteArray(char *compressed, int index, int value) {
    char mask = 255; 
    int bitBytes = 8;

    while(value != 0) {
        appendBitsToByte(compressed, index, value & mask, bitBytes)
        index += 1;
        value = value >> 8;
    }
}

void appendBitsToByte(char *compressed, int index, int value, int shift) {
    compressed[index] = compressed[index] << shift + value;
}

int maskLeastSigDigits(int value, int valueBits, int sigBitsNeeded) {
    int mask = (1 << (valueBits - sigBitsNeeded)) - 1;
    return value & mask;
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

void writeToFile(char *filename, char *text) {
    FILE *file = fopen(filename, "w");

    int results = fputs(text, file);
    if (results == EOF) {
        printf("Failed write");
    }
    fclose(file);
}

int main() {
    
    char *filename = "test.txt";
    huffmanEncode(filename);

    return 0;
}