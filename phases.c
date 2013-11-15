/*
 * CMPUT 481 Assignment 2
 *
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#include <stdlib.h>
#include <stdio.h>	/* printf() */
#include <unistd.h>	/* sleep() */
#include <string.h>	/* memset() */
#include "mpi.h"

#include "macros.h"
#include "phases.h"
#include "args.h"

/*
 * Run the PSRS algorithm with timings.
 */
int run(struct timing *times, struct arguments *args, int rank, int size) {
	int i;
	intArray *data = NULL;
	intArray *local = NULL;
	intArray *samples = NULL;
	intArray *pivots = NULL;
	intArray **partitions = NULL;
	intArray *partitionsHead = NULL;
	intArray *finalList = NULL;

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
	
	samples = calloc(1, sizeof(intArray));
	/* each processor takes p samples */
	samples->size = size;
	samples->arr = calloc(samples->size, sizeof(int));
	
	pivots = calloc(1, sizeof(intArray));
	/* select p-1 privots from samples */
	pivots->size = size - 1;
	pivots->arr = calloc(pivots->size, sizeof(int));

	/* generate list of integers */
	/* all procs need access to data->arr even it is is NULL */
	data = calloc(1, sizeof(intArray));
	MASTER {
		data->size = args->nElem;
		data->arr = gen_rand_list(data->size, args->seed);
	}

	partitions = calloc(size, sizeof(intArray *));
	partitionsHead = calloc(1, sizeof(intArray));
	finalList = calloc(1, sizeof(intArray));
	
	/* Phase 1: partition and sort local data */
	MASTER { times->tPhase1S = MPI_Wtime(); }
	phase_1(data, local, samples, interval);
	MASTER { times->tPhase1E = MPI_Wtime(); }

#if DEBUG
	printf("%d locals: ", rank);
	for (i = 0; i < local->size; i++) {
		printf("%d, ", local->arr[i]);
	}
	printf("\n");
#endif

	/* Phase 2: find pivots then partition */
	MASTER { times->tPhase2S = MPI_Wtime(); }
	phase_2(rank, size, samples, pivots, local, partitions, partitionsHead);
	MASTER { times->tPhase2E = MPI_Wtime(); }

	/* Phase 3: exchange partitions */
	MASTER { times->tPhase3S = MPI_Wtime(); }
	phase_3(rank, size, &partitions, &partitionsHead);
	MASTER { times->tPhase3E = MPI_Wtime(); }
	
	/* Phase 4: merge partitions */
	MASTER { times->tPhase4S = MPI_Wtime(); }
	phase_4(rank, size, partitions, finalList);
	MASTER { times->tPhase4E = MPI_Wtime(); }


	/* gather data */


	/* concatenate lists */

	/* end timer */
	MASTER {
		times->tEnd = MPI_Wtime();
	}
	
	free(data->arr);
	free(data);
	free(local->arr);
	free(local);
	free(samples->arr);
	free(samples);
	free(pivots->arr);
	free(pivots);
	free(partitionsHead->arr);
	free(partitionsHead);
	for (i = 0; i < size; i++) {
		free(partitions[i]);
	}
	free(partitions);

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
 *     p+k, 2p+k, 3p+k, ... , (p-1)+k
 *     where k = floor(p/2)
 * Each processor receives a copy of the pivots
 * Each processor makes p partitions from their local data
 */
void phase_2(int rank, int size, intArray *samples, intArray *pivots, intArray *local, intArray **partitions, intArray *partitionsHead) {
	intArray *gatheredSamples = NULL;
	/* loop variables */
	int i = 0;
	int j = 0;
	
	/* implicit floor because these are integers */
	int k = size / 2;

	/* only MASTER needs to collect all samples */
	gatheredSamples = calloc(1, sizeof(intArray));
	MASTER {
		gatheredSamples->size = size * size;
		gatheredSamples->arr = calloc(gatheredSamples->size, sizeof(int));
	}
	
#if DEBUG
	printf("%d samples: ", rank);
	for (i = 0; i < samples->size; i++) {
		printf("%d, ", samples->arr[i]);
	}
	printf("\n");
#endif

	/* gather samples on MASTER */
	MPI_Gather(
	    (void*)(samples->arr),
	    samples->size,
	    MPI_INT,
	    (void*)(gatheredSamples->arr),
	    /* recvcount is size sent from a single process */
	    samples->size,
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);

	MASTER {
#if DEBUG
		printf("%d gathered samples: ", rank);
		for (i = 0; i < gatheredSamples->size; i++) {
			printf("%d, ", gatheredSamples->arr[i]);
		}
		printf("\n");
#endif

		/* sort samples */
		// TODO check sorting alg
		qsort(gatheredSamples->arr, gatheredSamples->size, sizeof(int), &compare);

		/* select p-1 pivots */
		for (i = 0; i < pivots->size; i++) {
			pivots->arr[i] = gatheredSamples->arr[((i+1) * size) + k];
		}

#if DEBUG
		printf("%d pivots before bcast: ", rank);
		for (i = 0; i < pivots->size; i++) {
			printf("%d, ", pivots->arr[i]);
		}
		printf("\n");
#endif
	}

	/* done with gatheredSamples */

	free(gatheredSamples->arr);
	free(gatheredSamples);
	
	/* broadcast pivots */
	MPI_Bcast(
	    (void*)pivots->arr,
	    pivots->size,
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);

#if DEBUG
	printf("%d pivots: ", rank);
	for (i = 0; i < pivots->size; i++) {
		printf("%d, ", pivots->arr[i]);
	}
	printf("\n");
#endif

	i = 0;
	
	int pivot = 0;
	int index = 0;
	int partitionSize = 0;

	/* there are size-1 pivots for size number of partitions */
	for (i = 0; i < pivots->size; i++) {
		pivot = pivots->arr[i];

		for (j = index; j < local->size; j++) {
			if (local->arr[j] < pivot) {
				partitionSize++;
			} else {
				break;
			}
		}

		/* allocate for partition */
		// TODO realloc breaks previous arr pointers
		partitions[i] = calloc(1, sizeof(intArray));
		partitions[i]->size = partitionSize;
		partitionsHead->size += partitions[i]->size;
		partitionsHead->arr = realloc(partitionsHead->arr, partitionsHead->size * sizeof(int));
		partitions[i]->arr = partitionsHead->arr + partitionsHead->size - partitions[i]->size;
		memset(partitions[i]->arr, 0, partitions[i]->size * sizeof(int));

		/* copy values to partition */
		while (partitionSize > 0) {
			partitions[i]->arr[partitionSize - 1] = local->arr[partitionSize + index - 1];
			partitionSize--;
		}

		index = j;
		/* skip pivots */
		// TODO we don't want to skip duplicates here
		while (local->arr[index] == pivot) {
			index++;
		}
	}

#if DEBUG
	printf("%d all pivot partitions but last done\n", rank);
#endif

	/* add remaining values to final parition */
	/* using i from loop above */
	partitions[i] = calloc(1, sizeof(intArray));
	partitions[i]->size = local->size - index;

	partitionsHead->size += partitions[i]->size;
	partitionsHead->arr = realloc(partitionsHead->arr, partitionsHead->size * sizeof(int));
	partitions[i]->arr = partitionsHead->arr + partitionsHead->size - partitions[i]->size;
	memset(partitions[i]->arr, 0, partitions[i]->size * sizeof(int));

	for (j = index; j < local->size; j++) {
		// add to final partition
		partitions[i]->arr[j - index] = local->arr[j];
	}

#if DEBUG
	printf("%d final pivot parition done\n", rank);

	for (i = 0; i < size; i++) {
		printf("%d parition for pivot %d: ", rank, i);
		for (j = 0; j < partitions[i]->size; j++) {
			printf("%d, ", partitions[i]->arr[j]);
		}
		printf("\n");
	}
	printf("DONE PHASE 2\n");
#endif
	return;
}

/*
 * Phase 3
 *
 * Each porcessor i keeps the ith partition and sends the jth partition to the
 * jth processor
 */
void phase_3(int rank, int size, intArray ***partitionsPtr, intArray **partitionsHeadPtr) {
	int i,j = 0;
	int totalPartSize = 0;
	int *rdispls = NULL;
	int *sdispls = NULL;
	int offset = 0;
	intArray **partitions = NULL;
	intArray *partitionsHead = NULL;

	intArray *partSizes = NULL;
	intArray *newPartSizes = NULL;
	intArray **newPartitions = NULL;
	intArray *newPartitionsHead = NULL;

	partitions = *partitionsPtr;
	partitionsHead = *partitionsHeadPtr;

	/* buffer to exchange partition sizes */
	partSizes = calloc(1, sizeof(intArray));
	partSizes->size = size;
	partSizes->arr = calloc(partSizes->size, sizeof(int));
	newPartSizes = calloc(1, sizeof(intArray));
	newPartSizes->size = size;
	newPartSizes->arr = calloc(newPartSizes->size, sizeof(int));

	/* buffers for MPI_Alltoallv call */
	sdispls = calloc(size, sizeof(int));
	rdispls = calloc(size, sizeof(int));

	/* new partitions (from other processors) */
	newPartitions = calloc(size, sizeof(intArray*));
	newPartitionsHead = calloc(1, sizeof(intArray*));

	/* set local sizes */
	offset = 0;
	for (i = 0; i < size; i++) {
		partSizes->arr[i] = partitions[i]->size;
		sdispls[i] = offset;
		offset += partSizes->arr[i];
	}

#if DEBUG
	printf("%d local partition sizes: ", rank);
	for (i = 0; i < partSizes->size; i++) {
		printf("%d, ", partSizes->arr[i]);
	}
	printf("\n");
#endif

	/* exchange partition sizes with all others */
	MPI_Alltoall(
	    partSizes->arr,
	    1,
	    MPI_INT,
	    newPartSizes->arr,
	    1,
	    MPI_INT,
	    MPI_COMM_WORLD);

#if DEBUG
	printf("%d exchanged partition sizes: ", rank);
	for (i = 0; i < newPartSizes->size; i++) {
		printf("%d, ", newPartSizes->arr[i]);
	}
	printf("\n");
#endif

	/* calculate total memory needed */
	for (i = 0; i < newPartSizes->size; i++) {
		totalPartSize += newPartSizes->arr[i];
	}

	/* allocate memory for each parition */
	newPartitionsHead->arr = calloc(totalPartSize, sizeof(int));
	offset = 0;
	for (i = 0; i < newPartSizes->size; i++) {
		newPartitions[i] = calloc(1, sizeof(intArray));
		newPartitions[i]->size = newPartSizes->arr[i];
		newPartitions[i]->arr = newPartitionsHead->arr + offset;
		rdispls[i] = offset;
		offset += newPartSizes->arr[i];
	}

#if DEBUG
	for (i = 0; i < newPartSizes->size; i++) {
		printf("%d expected partition size from %d: %d\n", rank, i, newPartitions[i]->size);
	}
#endif

	/* exchange partitions */
	MPI_Alltoallv(
	    partitionsHead->arr,
	    partSizes->arr,
	    sdispls,
	    MPI_INT,
	    newPartitionsHead->arr,
	    newPartSizes->arr,
	    rdispls,
	    MPI_INT,
	    MPI_COMM_WORLD);

#if DEBUG
	for (i = 0; i < size; i++) {
		printf("%d exchanged parition from %d: ", rank, i);
		for (j = 0; j < newPartitions[i]->size; j++) {
			printf("%d, ", newPartitions[i]->arr[j]);
		}
		printf("\n");
	}
#endif

	/* swap old local paritions with new ones */
	free(partitionsHead->arr);
	free(partitionsHead);
	for (i = 0; i < size; i++) {
		free(partitions[i]);
	}
	free(partitions);
	*(partitionsHeadPtr) = newPartitionsHead;
	*(partitionsPtr) = newPartitions;

	/* clean up */
	free(sdispls);
	free(rdispls);
	free(partSizes->arr);
	free(partSizes);
	free(newPartSizes->arr);
	free(newPartSizes);
}

/*
 * Merge the local partitions
 */
void phase_4(int rank, int size, intArray **partitions, intArray *finalList) {

	int i = 0;
	int j = 0;
	int partWithMin = 0;
	int minVal = 0;
	int *indices = NULL;
	int finalBuzzSize = 0;
	/* do a k-way merge of already sorted lists */

#if DEBUG
	for (i = 0; i < size; i++) {
		printf("%d P4 parition from %d: ", rank, i);
		for (j = 0; j < partitions[i]->size; j++) {
			printf("%d, ", partitions[i]->arr[j]);
		}
		printf("\n");
	}
#endif
	for (i = 0; i < size; i++) {
		finalList->size += partitions[i]->size;
	}

	finalList->arr = calloc(finalList->size, sizeof(int));

	indices = calloc(size, sizeof(int));

	for (i = 0; i < finalList->size; i++) {
		for (j = 0; j < size; j++) {
			if (indices[j] < partitions[j]->size) {
				partWithMin = j;
				minVal = partitions[j]->arr[indices[j]];
			}
		}

		for (j = 0; j < size; j++) {
			if (indices[j] >= partitions[j]->size) {
				continue;
			}

			if (partitions[j]->arr[indices[j]] < minVal) {
				minVal = partitions[j]->arr[indices[j]];
				partWithMin = j;
			}
		}

		finalList->arr[i] = minVal;
		indices[partWithMin]++;
	}

#if DEBUG
	printf("%d final list: ", rank);
	for (i = 0; i < finalList->size; i++) {
		printf("%d, ", finalList->arr[i]);
	}
	printf("\n");
#endif
	return;
}

int compare(const void *x, const void *y) {
	int a = *(int*)x;
	int b = *(int*)y;

	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}
