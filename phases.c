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
#include "args.h"

/*
 * Run the PSRS algorithm with timings.
 */
int run(struct timing *times, struct arguments *args, int rank, int size) {
	int *data = NULL;
	int *local = NULL;
	int localCount = args->nElem / size;

	/* start timer */
	MASTER {
		times->tStart = MPI_Wtime();
	}

	/* generate list of integers */
	MASTER {
		data = gen_rand_list(args->nElem, args->seed);
	}

	/* split data into p partitions and scatter partitions */
	local = calloc(localCount, sizeof(int));
	scatter(data, local, localCount);

	int i;
	for (i = 0; i < localCount; i++) {
		printf("%d, ", local[i]);
	}
	printf("\n");

	/* Phase 1: sort local data */

	/* Phase 2: find pivots then partition */
	/* Phase 3: exchange partitions */
	/* Phase 4: merge partitions */

	/* gather data */
	/* concatenate lists */

	/* end timer */
	MASTER {
		times->tEnd = MPI_Wtime();
	}
	
	free(data);
	free(local);

	return 0;
}

/*
 * Generate list of appropriate size from random seed.
 *
 * Return a pointer to the list.
 * Be sure to free the pointer once no longer needed.
 */
int *gen_rand_list(int nElem, int seed) {
	int *elem = NULL;
	elem = malloc(nElem * sizeof(int));

	/* seed random number genrator */
	srandom(seed);

	int i;
	for (i = 0; i < nElem; i++) {
		//elem[i] = random();
		elem[i] = i;
	}

	return elem;
}

/*
 * Partition data into p parts and scatter parts to other processes
 * int the communicator.
 */
void scatter(int *data, int *local, int count) {
	MPI_Scatter(
	    (void*)data,
	    count,
	    MPI_INT,
	    (void*)local,
	    count,
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);
}
