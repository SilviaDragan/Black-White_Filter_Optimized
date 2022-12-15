build: bwfilter-serial.c
	gcc bwfilter-serial.c -o serial -Wall -Wextra
	gcc bwfilter-pthreads.c -o threads -lpthread -lm -Wall
	mpicc bwfilter-mpi.c -o bwmpi

run_serial:
	./serial

run_mpi:
	mpirun -np 4 ./bwmpi

# run_openmp:

run_pthreads:
	./threads

# run_hybrid:

clean:
	rm serial threads bwmpi