#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>

const int dec = 100000;


int main(int argc, char **argv) {
    if (argc < 2) {
        return 1;
    }

    int32_t dim = strtol(argv[1], NULL, 10);

    FILE * AFile = fopen ("A" , "wb");
    FILE * BFile = fopen ("B" , "wb");

    int32_t** A = (int32_t**)malloc(dim*sizeof(int32_t*));
    int32_t** B = (int32_t**)malloc(dim*sizeof(int32_t*));

    for (int i=0; i<dim; i++) {
        A[i] = (int32_t*)malloc(dim*sizeof(int32_t));
        B[i] = (int32_t*)malloc(dim*sizeof(int32_t));

        for (int j=0; j<dim; j++) A[i][j] = rand()/dec;
        for (int j=0; j<dim; j++) B[i][j] = rand()/dec;
    }

    fwrite(&dim, sizeof(dim), 1, AFile);
    fwrite(&dim, sizeof(dim), 1, BFile);

    for (int i=0; i<dim; i++) fwrite(A[i], sizeof(*A[i]), dim, AFile);
    for (int i=0; i<dim; i++) fwrite(B[i], sizeof(*B[i]), dim, BFile);

    for (int i = 0; i < dim; i++) free(A[i]);
    for (int i = 0; i < dim; i++) free(B[i]);
    free(A);
    free(B);

    fclose(AFile);
    fclose(BFile);
}