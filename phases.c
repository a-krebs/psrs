/*
 * CMPUT 481 Assignment 2
 *
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#include <stdlib.h>
#include <unistd.h>	/* sleep() */
#include "mpi.h"

#include "macros.h"
#include "phases.h"

/*
 * Run the PSRS algorithm with timings.
 */
int run(struct timing *times, int rank, int size) {

	times->tStart = MPI_Wtime();

	MASTER {
		sleep(3);
	}

	times->tEnd = MPI_Wtime();
	return 0;
}

/*
 * Generate list of appropriate size from random seed.
 *
 * Return a pointer to the list.
 * Be sure to free the pointer once no longer needed.
 */
int *gen_rand_list(int seed) {
	return NULL;
}
