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
	int *samples = NULL;
	int localCount = args->nElem / size;
	int numSamples = args->nElem / (size * size);

	/* start timer */
	MASTER {
		times->tStart = MPI_Wtime();
	}

	/* initialize local buffers */
	local = calloc(localCount, sizeof(int));
	samples = calloc(numSamples, sizeof(int));

	/* generate list of integers */
	MASTER {
		data = gen_rand_list(args->nElem, args->seed);
	}

	/* Phase 1: partition and sort local data */
	MASTER { times->tPhase1S = MPI_Wtime(); }
	phase_1(data, local, localCount, samples, numSamples);
	MASTER { times->tPhase1E = MPI_Wtime(); }

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

/*
 * Phase 1
 *
 * Each processor is assigned n/p items.
 * Each processor sorts their portion of the items using quicksort.
 * Each processor takes regular samples of their sorted local data.
 */
void phase_1(
    int *data, int *local, int localCount, int *samples, int numSamples) {
	/* split data into p partitions and scatter to other pocesses */
	scatter(data, local, localCount);

	/* sort local data */
	// TODO make sure we're actually using quicksort
	qsort(local, localCount, sizeof(int), &compare);

	/* take samples */
	int i;
	for (i = 0; i < numSamples; i++) {
		samples[i] = local[i*numSamples + 1];
	}

	return;
}

int compare(const void *x, const void *y) {
	int a = *(int*)x;
	int b = *(int*)y;

	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}
