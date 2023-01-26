#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

const int32_t MAX = 2147483646;
const int32_t MIN = 0;


int main(int argc, char **argv) {
    if (argc < 2) {
        return 1;
    }

    int32_t size = strtol(argv[1], NULL, 10);
    int32_t *arr = malloc(size*sizeof(int32_t));
    for (int32_t i = 0; i < size; i++) arr[i] = rand() % (MAX - MIN + 1) + MIN;

    FILE *file = fopen ("A" , "wb");
    fwrite(&size, sizeof(size), 1, file);
    fwrite(arr, sizeof(int32_t), size, file);
    fclose(file);

    free(arr);
}