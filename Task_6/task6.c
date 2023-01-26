#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>


int32_t NDIMS = 3;


void create_subarray(int32_t **mrx, int32_t step, int32_t i_block, int32_t j_block, int32_t *res, int32_t rank) {
    if (rank) return;
    for (int i = 0; i < step; i++) {
        for (int j = 0; j < step; j++) {
            res[i*step+j] = mrx[i_block*step+i][j_block*step+j];
        }
    }
}


int32_t main(int32_t argc, char** argv) {
    if (argc < 4) return 1;

    MPI_Init(&argc, &argv);

    int32_t rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int32_t side = (int32_t)(cbrt(size));
    int32_t dims[] = {side, side, side};
    int32_t periods[] = {0, 0, 0};
    int32_t coords[NDIMS];

    MPI_Comm grid;
    MPI_Cart_create(MPI_COMM_WORLD, NDIMS, dims, periods, 1, &grid);
    MPI_Comm_rank(grid, &rank);
    MPI_Cart_coords(grid, rank, NDIMS, coords);

    int32_t xc = coords[0], yc = coords[1], zc = coords[2];

    int32_t **aMrx;
    int32_t **bMrx;

    int32_t step, dim;

    if (!rank) {
        int32_t a_mrxdim, b_mrxdim;

        FILE* AFile = fopen(argv[1] , "rb");
        FILE* BFile = fopen(argv[2] , "rb");        

        fread(&a_mrxdim, sizeof(a_mrxdim), 1, AFile);
        fread(&b_mrxdim, sizeof(b_mrxdim), 1, BFile);

        if (a_mrxdim != b_mrxdim) MPI_Abort(grid, -1);

        dim = a_mrxdim;

        aMrx = (int32_t**)malloc(dim*sizeof(int32_t*));
        bMrx = (int32_t**)malloc(dim*sizeof(int32_t*));

        for (int i = 0; i < dim; i++) {
            aMrx[i] = (int32_t*)malloc(dim*sizeof(int32_t));
            bMrx[i] = (int32_t*)malloc(dim*sizeof(int32_t));

            fread(aMrx[i], sizeof(*aMrx[i]), dim, AFile);
            fread(bMrx[i], sizeof(*bMrx[i]), dim, BFile);
        }

        step = dim/side;

        fclose(AFile);
        fclose(BFile);
    }

    MPI_Bcast(&step, 1, MPI_INT, 0, grid);
    int32_t *a_block = malloc(step*step*sizeof(int32_t));
    int32_t *b_block = malloc(step*step*sizeof(int32_t));

    int32_t *a_blocks_flattened = malloc(dim*dim*sizeof(int32_t));
    int32_t *b_blocks_flattened = malloc(dim*dim*sizeof(int32_t));

    MPI_Request *a_requests = malloc(side*side*sizeof(MPI_Request));
    MPI_Request *b_requests = malloc(side*side*sizeof(MPI_Request));

    MPI_Status *a_statuses = malloc(side*side*sizeof(MPI_Status));
    MPI_Status *b_statuses = malloc(side*side*sizeof(MPI_Status));

    if (!rank) {
        for (int i = 0; i < side; i++) {
            for (int j = 0; j < side; j++) {
                int dest_rank;
                int coord[3] = {i, j, 0};
                MPI_Cart_rank(grid, coord, &dest_rank);

                int displ = (i*side + j)*step*step;

                create_subarray(aMrx, step, i, j, a_blocks_flattened+displ, rank);
                create_subarray(bMrx, step, i, j, b_blocks_flattened+displ, rank);

                MPI_Isend(a_blocks_flattened+displ, step*step, MPI_INT, dest_rank, 33, grid, &a_requests[i*side + j]);
                MPI_Isend(b_blocks_flattened+displ, step*step, MPI_INT, dest_rank, 44, grid, &b_requests[i*side + j]);
            }
        }

        for (int i = 0; i < dim; i++) {
            free(aMrx[i]);
            free(bMrx[i]);
        }
        free(aMrx);
        free(bMrx);
    }

    if (!zc) {
        MPI_Recv(a_block, step*step, MPI_INT, 0, 33, grid, MPI_STATUS_IGNORE);
        MPI_Recv(b_block, step*step, MPI_INT, 0, 44, grid, MPI_STATUS_IGNORE);
    }

    if (!rank) {
        MPI_Waitall(side*side, a_requests, a_statuses);
        MPI_Waitall(side*side, b_requests, b_statuses);
    }

    free(a_blocks_flattened);
    free(b_blocks_flattened);

    if (!zc) {
        for (int j = 0; j < side; j++) {
            int a_dest_rank, b_dest_rank;
            int a_dest_coord[3] = {xc, j, yc};
            int b_dest_coord[3] = {j, yc, xc};
            MPI_Cart_rank(grid, a_dest_coord, &a_dest_rank);
            MPI_Cart_rank(grid, b_dest_coord, &b_dest_rank);

            MPI_Isend(a_block, step*step, MPI_INT, a_dest_rank, 66, grid, &a_requests[j]);
            MPI_Isend(b_block, step*step, MPI_INT, b_dest_rank, 77, grid, &b_requests[j]);
        }
        
    }

    int a_src_rank, b_src_rank;
    int a_src_coord[3] = {xc, zc, 0};
    int b_src_coord[3] = {zc, yc, 0};
    MPI_Cart_rank(grid, a_src_coord, &a_src_rank);
    MPI_Cart_rank(grid, b_src_coord, &b_src_rank);

    MPI_Recv(a_block, step*step, MPI_INT, a_src_rank, 66, grid, MPI_STATUS_IGNORE);
    MPI_Recv(b_block, step*step, MPI_INT, b_src_rank, 77, grid, MPI_STATUS_IGNORE);

    if (!zc) {
        MPI_Waitall(side, a_requests, a_statuses);
        MPI_Waitall(side, b_requests, b_statuses);
    }

    int32_t *c_block = (int32_t*)malloc(step*step*sizeof(int32_t));

    for (int i = 0; i < step*step; i++) c_block[i] = 0;

    double t0 = MPI_Wtime();
    for (int i = 0; i < step; i++) {
        for (int j = 0; j < step; j++) {
            for (int k = 0; k < step; k++) {
                c_block[i*step+j] += a_block[i*step+k] * b_block[k*step+j];
            }
        }
    }
    double t = MPI_Wtime() - t0;

    int32_t *c_res_chunked = (int32_t*)malloc(step*step*side*sizeof(int32_t));

    MPI_Request c_request;
    MPI_Status c_status;
    
    int c_dest_rank;
    int c_dest_coord[3] = {xc, yc, 0};
    MPI_Cart_rank(grid, c_dest_coord, &c_dest_rank);
    MPI_Isend(c_block, step*step, MPI_INT, c_dest_rank, 99, grid, &c_request);

    if (!zc) {
        for (int k = 0; k < side; k++) {
            int c_src_rank;
            int c_src_coord[3] = {xc, yc, k};
            MPI_Cart_rank(grid, c_src_coord, &c_src_rank);
            MPI_Recv(c_res_chunked+k*step*step, step*step, MPI_INT, c_src_rank, 99, grid, MPI_STATUS_IGNORE);
        }

        for (int i = 0; i < step*step; i++) c_block[i] = 0;

        for (int i = 0; i < side; i++) {
            for (int j = 0; j < step*step; j++) {
                c_block[j] += c_res_chunked[i*step*step+j];
            }
        }
    }

    MPI_Wait(&c_request, &c_status);

    if (!zc) MPI_Isend(c_block, step*step, MPI_INT, 0, 11, grid, &c_request);

    double *timings = malloc(size*sizeof(double));
    MPI_Gather(&t, 1, MPI_DOUBLE, timings, 1, MPI_DOUBLE, 0, grid);

    if (!rank) {
        int32_t *c_res = (int32_t*)malloc(dim*dim*sizeof(int32_t));

        for (int i = 0; i < side; i++) {
            for (int j = 0; j < side; j++) {
                int c_res_src_rank;
                int c_res_src_coord[3] = {i, j, 0};
                MPI_Cart_rank(grid, c_res_src_coord, &c_res_src_rank);

                MPI_Recv(c_res+(i*side+j)*step*step, step*step, MPI_INT, c_res_src_rank, 11, grid, MPI_STATUS_IGNORE);
            }
        }

        int32_t **cMrx = (int32_t**)malloc(dim*sizeof(int32_t*));

        for (int i = 0; i < dim; i++) cMrx[i] = (int32_t*)malloc(dim*sizeof(int32_t));
        for (int i1 = 0; i1 < side; i1++)
            for (int j1 = 0; j1 < side; j1++)
                for (int i = 0; i < step; i++)
                    for (int j = 0; j < step; j++)
                        cMrx[i1*step + i][j1*step + j] = c_res[(i1*side + j1)*step*step + i*step + j];

        FILE* CFile = fopen(argv[3] , "wb");

        fwrite(&dim, sizeof(dim), 1, CFile);
        for (int i=0; i<dim; i++) fwrite(cMrx[i], sizeof(*cMrx[i]), dim, CFile);

        fclose(CFile);

        double tz = 0;
        for (int32_t i = 0; i < size; i++)
            if (timings[i] > tz) tz = timings[i];
        printf("Elapsed time %lf\n", tz);

        for (int i = 0; i < dim; i++) free(cMrx[i]);
        free(cMrx);
        free(c_res);
    }

    if (!zc) MPI_Wait(&c_request, &c_status);
    
    free(a_block);
    free(b_block);
    free(c_block);
    free(c_res_chunked);

    free(timings);

    free(a_requests);
    free(b_requests);

    MPI_Finalize();
    return 0;
}