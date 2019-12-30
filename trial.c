#include <stdio.h>
#include <stdlib.h>

int main() {
    int *p = malloc(sizeof(int) * 5);
    for(int i = 0; i < 5; i++) {
        *(p + i) = i;
    }
    for(int i = 0; i < 5; i++) {
        printf("%p\n", (p + i));
    }
    int t = *p;
    printf("%p\n", &t);

    return 0;
}