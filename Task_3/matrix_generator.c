#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


int DEBUG = 0;

int main(int argc, char **argv) {
    if (argc < 2) {
        return 1;
    }

    int32_t dim = strtol(argv[1], NULL, 10);

    FILE * AFile = fopen ("A" , "wb");
    FILE * BFile = fopen ("B" , "wb");
    FILE * CFile = fopen ("C" , "wb");

    int32_t** A = (int32_t**)malloc(dim*sizeof(int32_t*));
    int32_t** B = (int32_t**)malloc(dim*sizeof(int32_t*));
    int32_t** C = (int32_t**)malloc(dim*sizeof(int32_t*));

    for (int i=0; i<dim; i++) {
        A[i] = (int32_t*)malloc(dim*sizeof(int32_t));
        B[i] = (int32_t*)malloc(dim*sizeof(int32_t));
        C[i] = (int32_t*)malloc(dim*sizeof(int32_t));
        for (int j=0; j<dim; j++){
            A[i][j] = rand();
            B[i][j] = rand();
        }
    }

    if (DEBUG) {
        A[0][0] = 1;
        A[0][1] = 3;
        A[1][0] = 5;
        A[1][1] = -1;

        B[0][0] = 4;
        B[0][1] = -2;
        B[1][0] = 1;
        B[1][1] = 1;
    }

    fwrite(&dim, sizeof(dim), 1, AFile);
    fwrite(&dim, sizeof(dim), 1, BFile);
    fwrite(&dim, sizeof(dim), 1, CFile);

    for (int i=0; i<dim; i++) {
        fwrite(A[i], sizeof(*A[i]), dim, AFile);
        fwrite(B[i], sizeof(*B[i]), dim, BFile);
        fwrite(C[i], sizeof(*C[i]), dim, CFile);
    }

    for (int i = 0; i < dim; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);

    fclose(AFile);
    fclose(BFile);
    fclose(CFile);
}