/*
 * CMPUT 481 Assignment 2
 *
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#include <stdlib.h>
#include <stdio.h>	/* printf() */
#include <unistd.h>	/* sleep() */
#include "mpi.h"

#include "macros.h"
#include "phases.h"
#include "args.h"

/*
 * Run the PSRS algorithm with timings.
 */
int run(struct timing *times, struct arguments *args, int rank, int size) {

	intArray *data = NULL;
	intArray *local = NULL;
	intArray *localSamples = NULL;
	intArray *gatheredSamples = NULL;
	intArray *pivots = NULL;

	int localCount = args->nElem / size;
	int interval = args->nElem / (size * size);

	/* start timer */
	MASTER {
		times->tStart = MPI_Wtime();
	}

	/* initialize local buffers */
	local = calloc(1, sizeof(intArray));
	local->size = localCount;
	local->arr = calloc(local->size, sizeof(int));
	localSamples = calloc(1, sizeof(intArray));
	/* each processor takes p localSamples */
	localSamples->size = size;
	localSamples->arr = calloc(localSamples->size, sizeof(int));
	pivots = calloc(1, sizeof(intArray));
	/* select p-1 privots from samples */
	pivots->size = size - 1;
	pivots->arr = calloc(pivots->size, sizeof(int));
	/* only MASTER needs to collect all samples */
	MASTER {
		gatheredSamples = calloc(1, sizeof(intArray));
		gatheredSamples->size = size * size;
		gatheredSamples->arr = calloc(
		    gatheredSamples->size, sizeof(int));
	}

	/* generate list of integers */
	/* all procs need access to data->arr even it is is NULL */
	data = calloc(1, sizeof(intArray));
	MASTER {
		data->size = args->nElem;
		data->arr = gen_rand_list(data->size, args->seed);
	}

	/* Phase 1: partition and sort local data */
	MASTER { times->tPhase1S = MPI_Wtime(); }
	phase_1(data, local, localSamples, interval);
	MASTER { times->tPhase1E = MPI_Wtime(); }

	int i;
	for (i = 0; i < local->size; i++) {
		printf("%d, ", local->arr[i]);
	}
	printf("\n");
	for (i = 0; i < localSamples->size; i++) {
		printf("%d, ", localSamples->arr[i]);
	}
	printf("\n");

	/* Phase 2: find pivots then partition */
	/* Phase 3: exchange partitions */
	/* Phase 4: merge partitions */

	/* gather data */
	/* concatenate lists */

	/* end timer */
	MASTER {
		times->tEnd = MPI_Wtime();
	}
	
	MASTER {
		free(data->arr);
		free(data);
		free(gatheredSamples->arr);
		free(gatheredSamples);
	}
	free(local->arr);
	free(local);
	free(localSamples->arr);
	free(localSamples);
	free(pivots->arr);
	free(pivots);

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
void phase_1(intArray *data, intArray *local, intArray *samples, int interval) {
	/* split data into p partitions and scatter to other pocesses */
	scatter(data->arr, local->arr, local->size);

	/* sort local data */
	// TODO make sure we're actually using quicksort
	qsort(local->arr, local->size, sizeof(int), &compare);

	/* take samples */
	int i;
	for (i = 0; i < local->size; i += interval) {
		samples->arr[i/interval] = local->arr[i];
	}

	return;
}

/*
 * Phase 2
 *
 * MASTER processor gathers and sorts the regular samples.
 * p-1 pivots are selected from the regular samples at indeces
 *     p+p, 2p+p, 3p+p, ... , (p-1)+p
 * Each processor receives a copy of the pivots
 * Each processor makes p partitions from their local data
 */
void phase_2(int rank, intArray *samples, intArray *pivots, intArray *local) {
}

int compare(const void *x, const void *y) {
	int a = *(int*)x;
	int b = *(int*)y;

	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}
