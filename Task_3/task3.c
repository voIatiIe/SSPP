#include <papi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


int DEBUG = 0;

void ijk(int, int32_t**, int32_t**, int32_t**);
void ikj(int, int32_t**, int32_t**, int32_t**);
void kij(int, int32_t**, int32_t**, int32_t**);
void jik(int, int32_t**, int32_t**, int32_t**);
void jki(int, int32_t**, int32_t**, int32_t**);
void kji(int, int32_t**, int32_t**, int32_t**);
void check(char*, int);

void handle_error(int retval, const char *str) {
    if (retval != PAPI_OK) {
        fprintf(
            stderr, 
            "%s: PAPI error %d: %s\n",
            str, 
            retval, 
            PAPI_strerror(retval)
        );
        exit(1);
    }
}

int main(int argc, char **argv) {
    if (argc < 5) return 1;

    int event_set = PAPI_NULL, retval;
    long long metrics[2];

    retval = PAPI_library_init(PAPI_VER_CURRENT);

    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI library init error!\n");
        exit(1);
    }


    int type = strtol(argv[4], NULL, 10);
    FILE* AFile = fopen (argv[1] , "rb");
    FILE* BFile = fopen (argv[2] , "rb");
    
    int32_t dim;
    fread(&dim, sizeof(dim), 1, AFile);
    fread(&dim, sizeof(dim), 1, BFile);

    int32_t** A = (int32_t**)malloc(dim*sizeof(int32_t*));
    int32_t** B = (int32_t**)malloc(dim*sizeof(int32_t*));
    int32_t** C = (int32_t**)malloc(dim*sizeof(int32_t*));

    for (int i=0; i<dim; i++) {
        A[i] = (int32_t*)malloc(dim*sizeof(int32_t));
        B[i] = (int32_t*)malloc(dim*sizeof(int32_t));
        C[i] = (int32_t*)malloc(dim*sizeof(int32_t));
        fread(A[i], sizeof(*A[i]), dim, AFile);
        fread(B[i], sizeof(*B[i]), dim, BFile);
        for (int j = 0; j < dim; j++) {
            C[i][j] = 0;
        }
    }

    fclose(AFile);
    fclose(BFile);

    FILE* CFile = fopen (argv[3] , "wb");

    handle_error(PAPI_create_eventset(&event_set), "create eventset");
    handle_error(PAPI_add_event(event_set, PAPI_L1_DCM), "add PAPI_L1_DCM");
    handle_error(PAPI_add_event(event_set, PAPI_L2_DCM), "add PAPI_L2_DCM");
    handle_error(PAPI_start(event_set), "papi start");
    switch (type) {
        case 0:
            ijk(dim, A, B, C);
            break;
        case 1:
            ikj(dim, A, B, C);
            break;
        case 2:
            kij(dim, A, B, C);
            break;
        case 3:
            jik(dim, A, B, C);
            break;
        case 4:
            jki(dim, A, B, C);
            break;
        case 5:
            kji(dim, A, B, C);
            break;
        default:
            break;
    }
    handle_error(PAPI_stop(event_set, metrics), "papi stop");
    fwrite(&dim, sizeof(dim), 1, CFile);
    for (int i = 0; i < dim; i++) {
        fwrite(C[i], sizeof(*C[i]), dim, CFile);
    }
    fclose(CFile);

    printf(
        "PAPI_L1_DCM = %lld\n"
        "PAPI_L2_DCM = %lld\n",
        metrics[0],
        metrics[1]
    );

    for (int i = 0; i < dim; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);

    if (DEBUG) check(argv[3], dim);
}


void ijk(int dim, int32_t** A, int32_t** B, int32_t** C) {
    int32_t sum = 0;
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            sum = 0;
            for (int k = 0; k < dim; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

void ikj(int dim, int32_t** A, int32_t** B, int32_t** C) {
    int32_t r;
    for (int i = 0; i < dim; i++) {
        for (int k = 0; k < dim; k++) {
            r = A[i][k];
            for (int j = 0; j < dim; j++) {
                C[i][j] += r * B[k][j];
            }
        }
    }
}

void kij(int dim, int32_t** A, int32_t** B, int32_t** C) {
    int32_t r;
    for (int k = 0; k < dim; k++) {
        for (int i = 0; i < dim; i++) {
            r = A[i][k];
            for (int j = 0; j < dim; j++) {
                C[i][j] += r * B[k][j];
            }
        }
    }
}

void jik(int dim, int32_t** A, int32_t** B, int32_t** C) {
    int32_t sum = 0;
    for (int j = 0; j < dim; j++) {
        for (int i = 0; i < dim; i++) {
            sum = 0;
            for (int k = 0; k < dim; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

void jki(int dim, int32_t** A, int32_t** B, int32_t** C) {
    for (int j = 0; j < dim; j++) {
        for (int k = 0; k < dim; k++) {
            int32_t r = B[k][j];
            for (int i = 0; i < dim; i++) {
                C[i][j] += A[i][k] * r;
            }
        }
    }
}

void kji(int dim, int32_t** A, int32_t** B, int32_t** C) {
    for (int k = 0; k < dim; k++) {
        for (int j = 0; j < dim; j++) {
            int32_t r = B[k][j];
            for (int i = 0; i < dim; i++) {
                C[i][j] += A[i][k] * r;
            }
        }
    }
}

void check(char* fname, int dim) {
    FILE* CFileRead = fopen (fname , "rb");
    int32_t** CRead = (int32_t**)malloc(dim*sizeof(int32_t*));
    for (int i = 0; i < dim; i++) {
        CRead[i] = (int32_t*)malloc(dim*sizeof(int32_t*));
        fread(CRead[i], sizeof(*CRead[i]), dim, CFileRead);
    }

    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            printf("%d\n", CRead[i][j]);
        }
    }

    fclose(CFileRead);
    for (int i = 0; i < dim; i++) {
        free(CRead[i]);
    }
    free(CRead);
}