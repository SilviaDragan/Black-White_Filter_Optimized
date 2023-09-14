# Optimization of Black and White Filter on .bmp Images using OpenMP, MPI, and Pthreads

## Selected Problem:
The proposed problem for parallelization involves applying a black & white filter to a .bmp image. The algorithm for applying the filter is based on calculating the arithmetic mean between the RGB values of the pixels in the initial image, represented as a matrix.

## Compilation Instructions:
Use the following command to compile all solutions:
make

## Approach 1: OpenMP
To run the OpenMP solution, execute: ./run_openmp

In this solution, we parallelize the traversal of rows in the pixel matrix during the calculation of the grayscale value of each pixel.

## Approach 2: MPI
To run the MPI solution, execute: ./run_mpi

In this solution, Process 0 handles reading and writing. After reading and processing information about the original image, Process 0 sends the information to worker processes. The worker processes calculate the grayscale values of the pixels in the assigned rows and then send the processed rows back to Process 0. Process 0 receives the processed rows from workers and assembles the matrix for display.

## Approach 3: Pthreads
To run the Pthreads solution, execute: run_pthreads
In this solution, each thread processes the assigned columns in the matrix.

## Approach 4: MPI-OpenMP (Hybrid solution)
To run the hybrid MPI-OpenMP solution, execute:  ./run_hybrid
This algorithm is similar to the MPI solution but includes parallelization of the traversal of assigned rows.
