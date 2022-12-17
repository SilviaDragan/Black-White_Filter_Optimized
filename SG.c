#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


#define MASTER 0

/*
MPI_Scatter(
    void* send_data,
    int send_count,
    MPI_Datatype send_datatype,
    void* recv_data,
    int recv_count,
    MPI_Datatype recv_datatype,
    int root,
    MPI_Comm communicator)


*/




int main (int argc, char **argv) {
    int rank, proc, a;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);

    int **matrix;
    int *send_arr = calloc(sizeof(int *), 4*4);;
    int *result_arr = calloc(sizeof(int *), 4*4);;

    if (rank == MASTER) { 
        matrix = calloc(sizeof(int *), 4);
        for (int i = 0; i < 4; i++) {
            matrix[i] = calloc(sizeof(int), 4);
        }

        a = 0;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                matrix[i][j] = a;
                a++;
            }
        }

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                printf("%d ", matrix[i][j]);
            }
            printf("\n");
        }

        // inline matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                send_arr[4*i + j] = matrix[i][j];
            }
        }
        printf("\n");

        for (int i = 0; i < 4*4; i++) { 
            printf("%d ", send_arr[i]);
        }

        printf("\n");

    }
    int nmin = 16 / proc;
    int nextra = 16 % proc;
    int k = 0;
    int *sendcounts = calloc(sizeof(int), proc);
    int *displs = calloc(sizeof(int), proc);

    for (int i = 0; i < proc; i++) {
        if (i < nextra){
            sendcounts[i] = nmin + 1;
        } 
        else {
            sendcounts[i] = nmin;
        }
        displs[i] = k;
        k += sendcounts[i];
    }
    int recvcount = sendcounts[rank]; // ??
    int *recv_arr = calloc(sizeof(int), recvcount);
    printf("inainte de scatter sunt cu rank %d\n", rank);

    MPI_Scatterv(send_arr, sendcounts, displs, MPI_INT, recv_arr, recvcount, MPI_INT, MASTER, MPI_COMM_WORLD);

    printf("dupa scatter sunt cu rank %d\n", rank);
    for (int i = 0; i < recvcount; i++) {
        recv_arr[i] += 1;
    }

    MPI_Gatherv(recv_arr, recvcount, MPI_INT, result_arr, sendcounts, displs, MPI_INT, MASTER, MPI_COMM_WORLD);
    printf("dupa gather sunt cu rank %d\n", rank);

    if (rank == MASTER) {
        for (int i = 0; i < 16; i++) {
            printf("%d ", result_arr[i]);
        }
    }

    MPI_Finalize();
    return 0;
}


/*
MPI_Scatterv(send_arr, sendcounts, displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype,
                 int root, MPI_Comm comm)
*/