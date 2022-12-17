build: bwfilter-serial.c
	gcc bwfilter-serial.c -o serial -Wall -Wextra
	gcc bwfilter-pthreads.c -o threads -lpthread -lm -Wall -std=gnu99
	mpicc bwfilter-mpi.c -o bwmpi
	# g++ bwfilter-openmp.cpp -o ./bwopenmp -fopenmp -lpthread
	# mpicc SG.c -o sg
	

run_serial:
	./serial

run_mpi:
	# mpirun -np 4 ./sg
	mpirun -np 4 ./bwmpi

# run_openmp:
	# ./bwopenmp

run_pthreads:
	./threads

# run_hybrid:

clean:
	rm serial