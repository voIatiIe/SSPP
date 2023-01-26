#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mpi.h"


int32_t main(int32_t argc, char** argv) {
    if (argc < 4) return 1;

    MPI_Init(&argc, &argv);

    int32_t rank, size, dim;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_File AFile;
    int32_t ierr = MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &AFile);
    if (ierr) printf("Couldn't open input matrix file");

    MPI_File_read(AFile, &dim, 1, MPI_INT32_T, MPI_STATUS_IGNORE);

    int32_t n_rows = dim/size;
    int32_t chunk_size = dim*n_rows;
    int32_t *chunk = malloc(chunk_size*sizeof(int32_t));


    MPI_Offset offset = (rank*chunk_size + 1)*sizeof(int32_t);
    MPI_File_read_at_all(AFile, offset, chunk, chunk_size, MPI_INT32_T, MPI_STATUS_IGNORE);

    int32_t *bvec = malloc(dim*sizeof(int32_t));

    if (!rank) {
        int32_t bvecdim;
        FILE* BFile = fopen(argv[2] , "rb");
        fread(&bvecdim, sizeof(bvecdim), 1, BFile);

        if (bvecdim != dim) {
            printf("Wrong input dimensions\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fread(bvec, sizeof(int32_t), dim, BFile);
        fclose(BFile);
    }

    MPI_Bcast(bvec, dim, MPI_INT32_T, 0, MPI_COMM_WORLD);

    int32_t *cvec_chunk = malloc(n_rows*sizeof(int32_t));

    double t0 = MPI_Wtime();
    for (int32_t i = 0; i < n_rows; i++) {
        int32_t sum = 0;
        for (int32_t j = 0; j < dim; j++) {
            sum += bvec[j]*chunk[i*dim + j];
        }
        cvec_chunk[i] = sum;
    }
    double t = MPI_Wtime() - t0;

    int32_t *cvec = (int32_t*)malloc(dim*sizeof(int32_t));
    MPI_Gather(cvec_chunk, n_rows, MPI_INT32_T, cvec, n_rows, MPI_INT32_T, 0, MPI_COMM_WORLD);

    if (!rank) {
        FILE* CFile = fopen(argv[3] , "wb");
        fwrite(&dim, sizeof(dim), 1, CFile);
        fwrite(cvec, sizeof(int32_t), dim, CFile);
        fclose(CFile);
    }

    double *timings = malloc(size*sizeof(double));
    MPI_Gather(&t, 1, MPI_DOUBLE, timings, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if (!rank) {
        double tz = 0;
        for (int32_t i = 0; i < size; i++) {
            if (timings[i] > tz) {tz = timings[i];}
        }
        printf("Elapsed time %lf\n", tz);
    }

    free(chunk);
    free(bvec);
    free(cvec_chunk);
    free(cvec);
    free(timings);
    MPI_File_close(&AFile);
    MPI_Finalize();
    return 0;
}
