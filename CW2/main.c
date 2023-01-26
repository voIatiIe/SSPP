#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


void my_bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    if (rank == root) {
        for (int i=0; i < size; i++) {
            if (i != root) MPI_Send(buffer, count, datatype, i, 0, comm);
        }
    }
    else MPI_Recv(buffer, count, datatype, root, 0, comm, MPI_STATUS_IGNORE);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        exit(1);
    }

    int data_size = atoi(argv[1]);
    int iter = atoi(argv[2]);
    int world_rank;
    int* data = (int*)malloc(sizeof(int) * data_size);
    double my_bcast_time = 0, mpi_bcast_time = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    for (int i = 0; i < iter; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        my_bcast_time -= MPI_Wtime();
        my_bcast(data, data_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        my_bcast_time += MPI_Wtime();

        MPI_Barrier(MPI_COMM_WORLD);
        mpi_bcast_time -= MPI_Wtime();
        MPI_Bcast(data, data_size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        mpi_bcast_time += MPI_Wtime();
    }
    MPI_Finalize();

    if (world_rank == 0) {
        printf("my_bcast time = %lf\n", my_bcast_time / iter);
        printf("MPI_Bcast time = %lf\n", mpi_bcast_time / iter);
    }

    free(data);
}