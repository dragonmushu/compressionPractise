#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compression.h"


/*
* Function Definitions
*/
void huffmanEncode(char *input, char *output);
void huffmanDecode(char *input, char *output);
CodeList * textToCharCodes(char *);
void charFrequency(char *, int *);
int obtainValidDictLength(int *);
CodeNode * frequencyToCodes(int *, int);
CodeList * buildHuffmanHeap(CodeList *);
void forwardHeapify(CodeList *, int); 
void backwardHeapify(CodeList *, int);
int findMinFreqIndex(CodeList *, int, int, int);
void swapCodes(CodeNode *, int, int);
CodeNode * buildHuffmanTree(CodeList *);
CodeNode heapPop(CodeList *);
void heapPush(CodeList *, CodeNode);
int * treeToCharDict(CodeNode *);
void codifyTree(CodeNode *, int *, int);
CodeList * duplicateHuffmanDictionary(CodeList *);
void freeHuffmanTree(CodeNode *);
int tagArg(char *);


void huffmanEncode(char *input, char *output) {
    char *text = readFromFile(input);
    CodeList *codeList = textToCharCodes(text);
    CodeList *codeHeap = duplicateCodeList(codeList);

    codeHeap = buildHuffmanHeap(codeHeap);
    CodeNode *root = buildHuffmanTree(codeHeap);
    int *charDict = treeToCharDict(root);
    compressAndWriteToFile(codeList, charDict, text, output);

    freeCodeList(codeHeap);
    freeCodeList(codeList);
    freeHuffmanTree(root);
    free(charDict);
    free(text);
}

void huffmanDecode(char *input, char *output) {
    decompressAndWriteToFile(input, output);     
}

CodeList * textToCharCodes(char *text) {
    int charDict[256] = {0};
    charFrequency(text, charDict);
    int length = obtainValidDictLength(charDict);
    CodeList *codes = malloc(sizeof(CodeList));
    codes->size = length;
    codes->root = frequencyToCodes(charDict, length);

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

CodeNode * frequencyToCodes(int *charDict, int length) {
    CodeNode *codes = malloc(sizeof(CodeNode) * length);
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

CodeList * buildHuffmanHeap(CodeList *codes) {
    int currentIndex = (codes->size - 1) / 2;
    while(currentIndex >= 0) {
        forwardHeapify(codes, currentIndex);
        currentIndex -= 1;
    }

    return codes;
}

void forwardHeapify(CodeList *codes, int current) {
    int left = current * 2 + 1;
    int right = current * 2 + 2;
    int minFreqIndex = findMinFreqIndex(codes, current, left, right);
    
    if(minFreqIndex == left) {
        swapCodes(codes->root, left, current);
        forwardHeapify(codes, left);
    }
    if(minFreqIndex == right) {
        swapCodes(codes->root, right, current);
        forwardHeapify(codes, right);
    }
}

void backwardHeapify(CodeList *codes, int current) {
    if(current != 0) {
        int parent = (current - 1) / 2;
        if(codes->root[current].freq < codes->root[parent].freq) {
            swapCodes(codes->root, current, parent);
            backwardHeapify(codes, parent);
        }
    }
}

int findMinFreqIndex(CodeList *codes, int current, int left, int right) {
    int leftFreq = codes->root[current].freq;
    int rightFreq = codes->root[current].freq;
    int currentFreq = codes->root[current].freq;

    if(left < codes->size) leftFreq = codes->root[left].freq;
    if(right < codes->size) rightFreq = codes->root[right].freq;

    if(leftFreq <= rightFreq && leftFreq < currentFreq) return left;
    if(rightFreq <= leftFreq && rightFreq < currentFreq) return right;
    return current;
} 

void swapCodes(CodeNode *codes, int firstIndex, int secondIndex) {
    CodeNode temp = codes[firstIndex];
    codes[firstIndex] = codes[secondIndex];
    codes[secondIndex] = temp;
}

CodeNode * buildHuffmanTree(CodeList *codes) {
    while (codes->size > 1) {
        CodeNode *min1 = malloc(sizeof(CodeNode));
        CodeNode *min2 = malloc(sizeof(CodeNode));
        *min1 = heapPop(codes);
        *min2 = heapPop(codes);

        CodeNode root;
        root.left = min2;
        root.right = min1;
        root.freq = min1->freq + min2->freq;
        root.key = -1;
        heapPush(codes, root);
    }

    CodeNode *root = malloc(sizeof(CodeNode));
    *root = heapPop(codes);
    return root;
}

CodeNode heapPop(CodeList *codes) {
    CodeNode top = codes->root[0];
    codes->size -= 1;
    if (codes->size != 0) {
        swapCodes(codes->root, 0, codes->size);
        forwardHeapify(codes, 0);
    }
    
    return top;
}

void heapPush(CodeList * codes, CodeNode code) {
    codes->root[codes->size] = code;
    backwardHeapify(codes, codes->size);
    codes->size += 1;
}

int * treeToCharDict(CodeNode *root) {
    int * charDict = malloc(sizeof(int) * 256);
    codifyTree(root, charDict, 1);

    return charDict;
}

void codifyTree(CodeNode *root, int *codes, int code) {
    if(root != NULL) {
        codifyTree(root->left, codes, (code << 1) + 1);
        if(root->key != -1) {
            int index = root->key;
            codes[index] = code;
        }
        codifyTree(root->right, codes, code << 1);
    }
}

void freeHuffmanTree(CodeNode *root) {
    if(root != NULL) {
        freeHuffmanTree(root->left);
        freeHuffmanTree(root->right);
        free(root);
    }
}

int tagArg(char *arg) {
    if(findStringSize(arg) != 2 || arg[0] != '-') return -1;
    
    if(arg[1] == 'c') return 0;
    if(arg[1] == 'd') return 1;
    
    return -1;
}

int main(int argc, char **argv) {
    
    int type = -1;
    if(argc != 4 || (type = tagArg(argv[1])) == -1) {
        printf("Invalid Arguments\n");
        return 0;
    } 
    if(type == 0) huffmanEncode(argv[2], argv[3]);
    else huffmanDecode(argv[2], argv[3]);
    return 0;
}