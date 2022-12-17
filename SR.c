#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


#define MASTER 0
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))



int main (int argc, char **argv) {
    int rank, proc, a;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);

    int **matrix;
    int *send_arr = calloc(sizeof(int *), 4*4);
    int *result_arr = calloc(sizeof(int *), 4*4);

    int height = 4;
    int width = 4;

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

        for (int p = 1; p < proc; p++) {
                // int start = p * (double)width / proc;
                // int end = MIN((p + 1) * (double) width / proc, width);

                // printf("start = %d, end = %d pt proces %d \n", start, end, p);

                MPI_Send(&height, 1, MPI_INT, p, 0, MPI_COMM_WORLD);
                MPI_Send(&width, 1, MPI_INT, p, 0, MPI_COMM_WORLD);
                MPI_Send(&(send_arr[0]), height * width, MPI_INT, p, 0, MPI_COMM_WORLD);
        }

    } else {
        MPI_Status status;
        int rheight, rwidth;
        MPI_Recv(&rheight, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&rwidth, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int start = rank * (double)width / proc;
        int end = MIN((rank + 1) * (double) width / proc, width);

        int **rmatrix = calloc(sizeof(int *), rheight);
        for (int i = 0; i < 4; i++) {
            rmatrix[i] = calloc(sizeof(int), rwidth);
        }
        int *recvArray = calloc(sizeof(int), rwidth*rheight);

        MPI_Recv(&(recvArray[0]), rheight * rwidth, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        printf("received array:\n");
        for (int i = 0; i < rheight * rwidth; i++) {
            printf("%d ", recvArray[i]);
        }
        printf("\n");

        // for (int i = 0; i < rheight; i++) {
        //     for (int j = 0; j < rheight; j++) {
        //         rmatrix[i][j] = recvArray[rheight * i + j];
        //     }
        // }

        for (int i = 0; i < rheight; i++) {
            for (int j = start; j < end; j++) {
                recvArray[height * i + j] += 1;
                // printf("i= %d, j= %d\n", i, j);
            }
        }

        //  for (int i = 0; i < rheight * rwidth; i++) {
        //     printf("%d ", recvArray[i]);
        // }
        // printf("\n");

        MPI_Send(&(recvArray[0]), height * width, MPI_INT, p, 0, MPI_COMM_WORLD);
        


    }
    


    


    // int recvcount = sendcounts[rank];
    // int *recv_arr = calloc(sizeof(int), recvcount);
    // printf("inainte de scatter sunt cu rank %d\n", rank);

    // MPI_Scatterv(send_arr, sendcounts, displs, MPI_INT, recv_arr, recvcount, MPI_INT, MASTER, MPI_COMM_WORLD);

    // printf("dupa scatter sunt cu rank %d\n", rank);
    // for (int i = 0; i < recvcount; i++) {
    //     recv_arr[i] += 1;
    // }

    // MPI_Gatherv(recv_arr, recvcount, MPI_INT, result_arr, sendcounts, displs, MPI_INT, MASTER, MPI_COMM_WORLD);
    // printf("dupa gather sunt cu rank %d\n", rank);

    // if (rank == MASTER) {
    //     for (int i = 0; i < 16; i++) {
    //         printf("%d ", result_arr[i]);
    //     }
    // }

    MPI_Finalize();
    return 0;
}


/*
MPI_Scatterv(send_arr, sendcounts, displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype,
                 int root, MPI_Comm comm)
*/