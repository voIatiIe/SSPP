#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


long get_time_us() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return (1.0e6 * tval.tv_sec + tval.tv_usec);
}


int cmpfunc(const void *a, const void *b) {
    return (*(int32_t*)a - *(int32_t*)b);
}


int main(int argc, char **argv) {
    if (argc < 1) {
        return 1;
    }
    FILE* inFile = fopen (argv[1] , "rb");

    int32_t inputSize; fread(&inputSize, sizeof(inputSize), 1, inFile);
    int32_t *inputArray = (int32_t*)malloc(inputSize*sizeof(int32_t));
    fread(inputArray, sizeof(*inputArray), inputSize, inFile);

    long t1 = get_time_us();
    qsort(inputArray, inputSize, sizeof(*inputArray), cmpfunc);
    printf("Qsort elapsed time(us):\t\t%ld\n", get_time_us() - t1);

    free(inputArray);
    fclose(inFile);
}