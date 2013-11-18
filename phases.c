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
	/* loop variables */
	int i;
	/* complete random dataset, only filled on MASTER */
	intArray *data = NULL;
	/* local data, samples, pivots*/
	intArray *local = NULL;
	intArray *samples = NULL;
	intArray *pivots = NULL;
	/* partitions and final sorted list */
	/* 
	 * partitions are in a contguous memory area so that the entire memory
	 * area can be passed into MPI_Alltoallv. The individual partition
	 * structs in the **partitions array each have their arr pointer point
	 * to a space within partitionsHead->arr. Free only the
	 * partitionsHead-arr when done (and each of the partitions[i] structs,
	 * but NOT partitions[i]>arr).
	 */
	intArray **partitions = NULL;
	intArray *partitionsHead = NULL;
	intArray *finalList = NULL;

	/* what is the interval at which to take samples? */
	int interval = args->nElem / (size * size);
	if (interval == 0) {
		interval = 1;
	}

	/* start timer */
	MASTER {
		times->tStart = MPI_Wtime();
	}

	/* initialize local buffers */
	local = calloc(1, sizeof(intArray));
	local->size = args->nElem / size;
	local->arr = calloc(local->size, sizeof(int));
	samples = calloc(1, sizeof(intArray));
	/* each processor takes p samples */
	samples->size = size;
	samples->arr = calloc(samples->size, sizeof(int));
	pivots = calloc(1, sizeof(intArray));
	/* select p-1 privots from samples */
	pivots->size = size - 1;
	pivots->arr = calloc(pivots->size, sizeof(int));
	partitions = calloc(size, sizeof(intArray *));
	partitionsHead = calloc(1, sizeof(intArray));
	finalList = calloc(1, sizeof(intArray));
	
	/* generate list of integers */
	/* all procs need access to data->arr even it is is NULL */
	data = calloc(1, sizeof(intArray));
	MASTER {
		data->size = args->nElem;
		data->arr = gen_rand_list(data->size, args->seed);
	}

	/* split data into p partitions and scatter to other pocesses */
	scatter_data(data->arr, local->arr, local->size);
	
	/* Phase 1: partition and sort local data */
	MASTER { times->tPhase1S = MPI_Wtime(); }
	phase_1(rank, data, local, samples, interval);
	MASTER { times->tPhase1E = MPI_Wtime(); }

	/* Phase 2: find pivots then partition */
	MASTER { times->tPhase2S = MPI_Wtime(); }
	phase_2(rank, size, samples, local, pivots, partitions, partitionsHead);
	MASTER { times->tPhase2E = MPI_Wtime(); }

	/* Phase 3: exchange partitions */
	MASTER { times->tPhase3S = MPI_Wtime(); }
	phase_3(rank, size, &partitions, &partitionsHead);
	MASTER { times->tPhase3E = MPI_Wtime(); }
	
	/* Phase 4: merge partitions */
	MASTER { times->tPhase4S = MPI_Wtime(); }
	phase_4(rank, size, partitions, finalList);
	MASTER { times->tPhase4E = MPI_Wtime(); }

#if GATHERFINAL
	/* gather data and print for verification */
	phase_5(rank, size, args->nElem, finalList, pivots);
#endif

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
	free(finalList->arr);
	free(finalList);

#if DEBUG
	printf("%d RUN DONE\n", rank);
#endif

	return 0;
}

/*
 * Print an intArray with a message
 */
void print_intArray(int rank, intArray *ia, char *message) {
	int i = 0;
	printf("%d %s: ", rank, message);
	for (i = 0; i < ia->size; i++) {
		printf("%d, ", ia->arr[i]);
	}
	printf("\n");
}

/*
 * Generate list of appropriate size from random seed.
 *
 * Return a pointer to the list.
 * Be sure to free the pointer once no longer needed.
 */
int *gen_rand_list(int nElem, int seed) {
	int *elem = NULL;
	int i = 0;
	elem = malloc(nElem * sizeof(int));

	/* seed random number genrator */
	srandom(seed);

	for (i = 0; i < nElem; i++) {
		elem[i] = (int)random();
	}

	return elem;
}

/*
 * Partition data into p parts and scatter parts to other processes
 * int the communicator.
 */
void scatter_data(int *data, int *local, int count) {
	MPI_Scatter(
	    data,
	    count,
	    MPI_INT,
	    local,
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
    int rank, intArray *data, intArray *local,
    intArray *samples, int interval) {

	int i = 0;

	/* sort local data */
	// TODO make sure we're actually using quicksort
	qsort(local->arr, local->size, sizeof(int), &compare);

	/* take samples */
	for (i = 0; i < local->size; i += interval) {
		samples->arr[i/interval] = local->arr[i];
	}

#if DEBUG
	print_intArray(rank, local, "locals");
#endif
	return;
}

/*
 * Gather samples from all processors to MASTER
 */
void gather_samples(
    int rank, int size, intArray* gatheredSamples, intArray *samples) {
	MASTER {
		gatheredSamples->size = size * size;
		gatheredSamples->arr = calloc(
		    gatheredSamples->size, sizeof(int));
	}
	
#if DEBUG
	print_intArray(rank, samples, "samples");
#endif

	/* gather samples on MASTER */
	MPI_Gather(
	    samples->arr,
	    samples->size,
	    MPI_INT,
	    gatheredSamples->arr,
	    /* recvcount is size sent from a single process */
	    samples->size,
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);

	return;
}

/*
 * Use gathered samples to select pivots
 * and broadcase them to other processes
 */
void broadcast_pivots(
    int rank, int size, intArray *gatheredSamples, intArray *pivots) {
	int i = 0;
	/* interval at which to select pivots
	 * (implicit floor because these are integers) */
	int k = size / 2;

	MASTER {
#if DEBUG
		print_intArray(rank, gatheredSamples, "gathered samples");
#endif

		/* sort samples */
		// TODO check sorting alg
		qsort(
		    gatheredSamples->arr, gatheredSamples->size,
		    sizeof(int), &compare);

		/* select p-1 pivots */
		for (i = 0; i < pivots->size; i++) {
			pivots->arr[i] = gatheredSamples->arr[((i+1)*size) + k];
		}

#if DEBUG
		print_intArray(rank, pivots, "pivots before broadcast");
#endif
	}

	/* broadcast pivots */
	MPI_Bcast(
	    pivots->arr,
	    pivots->size,
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);

#if DEBUG
	print_intArray(rank, pivots, "pivots after broadcast");
#endif

	return;
}

/*
 * Re-assign the arr pointers for the first numP partitions in partitions array.
 */
void reset_arr_pointers(
    intArray *partitionsHead, intArray **partitions, int numP) {

	int i = 0;
	int *currArr = partitionsHead->arr;

	for (i = 0; i < numP; i++) {
		partitions[i]->arr = currArr;
		currArr += partitions[i]->size;
	}

	return;
}



/*
 * Partition local data based on pivots
 */
void partition_data(
    int rank, int size, intArray *local, intArray *pivots,
    intArray **partitions, intArray *partitionsHead) {

	int i = 0;
	int j = 0;
	int k = 0;
	int pivot = 0;
	int index = 0;
	int partitionSize = 0;
	int repeatedPivot = 0;

	/* there are size-1 pivots for size number of partitions */
	for (i = 0; i < pivots->size; i++) {
		pivot = pivots->arr[i];
		repeatedPivot = 0;

		for (j = index; j < local->size; j++) {
			if (local->arr[j] < pivot) {
				partitionSize++;
			} else {
				/* include pivot if it repeats*/
				for (k = j + 1; k < local->size; k++) {
					if (local->arr[k] == pivot) {
						repeatedPivot++;
					} else {
						break;
					}
				}
				partitionSize += repeatedPivot;
				j += repeatedPivot;
				break;
			}
		}

		/* allocate for partition */
		partitions[i] = calloc(1, sizeof(intArray));
		partitions[i]->size = partitionSize;
		partitionsHead->size += partitions[i]->size;
		partitionsHead->arr = realloc(partitionsHead->arr, partitionsHead->size * sizeof(int));
		reset_arr_pointers(partitionsHead, partitions, i + 1);
		memset(partitions[i]->arr, 0, partitions[i]->size * sizeof(int));

		/* copy values to partition */
		while (partitionSize > 0) {
			partitions[i]->arr[partitionSize - 1] = local->arr[partitionSize + index - 1];
			partitionSize--;
		}

		/* set start index for next partition */
		index = j;
		// skip pivot
		if (local->arr[index] == pivot) {
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
 * Phase 2
 *
 * MASTER processor gathers and sorts the regular samples.
 * p-1 pivots are selected from the regular samples at indeces
 *     p+k, 2p+k, 3p+k, ... , (p-1)+k
 *     where k = floor(p/2)
 * Each processor receives a copy of the pivots
 * Each processor makes p partitions from their local data
 */
void phase_2(
    int rank, int size, intArray *samples, intArray *local, intArray *pivots,
    intArray **partitions, intArray *partitionsHead) {

	/* loop variables */
	intArray *gatheredSamples = NULL;

	/* gather samples on MASTER */
	gatheredSamples = calloc(1, sizeof(intArray));
	gather_samples(rank, size, gatheredSamples, samples);

	/* select and broadcase pivots */
	broadcast_pivots(rank, size, gatheredSamples, pivots);

	/* done with gatheredSamples */
	free(gatheredSamples->arr);
	free(gatheredSamples);

	/* partition local data based on pivots */
	partition_data(rank, size, local, pivots, partitions, partitionsHead);
	
	return;
}

/*
 * echange parition sizes with other processors
 */
void exchange_partition_sizes(
    int rank, intArray *partSizes, intArray *newPartSizes) {

#if DEBUG
	print_intArray(rank, partSizes, "local partition sizes");
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
	print_intArray(rank, newPartSizes, "exchanged partition sizes");
#endif
}

/*
 * Exchange partitions with other processes
 */
void exchange_partitions(
    int rank, int size, 
    intArray *partSizes, intArray *newPartSizes,
    intArray **partitions, intArray *partitionsHead,
    intArray **newPartitions, intArray *newPartitionsHead) {

	int i = 0;
#if DEBUG
	int j = 0;
#endif
	int offset = 0;
	int totalPartSize = 0;
	int *rdispls = NULL;
	int *sdispls = NULL;

	/* buffers for MPI_Alltoallv call */
	sdispls = calloc(size, sizeof(int));
	rdispls = calloc(size, sizeof(int));
	
	/* get partition offset in local partitionsHead array */
	offset = 0;
	for (i = 0; i < size; i++) {
		sdispls[i] = offset;
		offset += partSizes->arr[i];
	}

	/* calculate total memory needed for incoming partitions*/
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
		/* while we're looping, keep track of
		 * partition offset in newPartitionsHead */
		rdispls[i] = offset;
		offset += newPartSizes->arr[i];
	}

#if DEBUG
	for (i = 0; i < newPartSizes->size; i++) {
		printf("%d expected partition size from %d: %d\n",
		    rank, i, newPartitions[i]->size);
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
	free(sdispls);
	free(rdispls);
}

/*
 * Phase 3
 *
 * Each porcessor i keeps the ith partition and sends the jth partition to the
 * jth processor
 */
void phase_3(
    int rank, int size, intArray ***partitionsPtr,
    intArray **partitionsHeadPtr) {

	int i = 0;
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

	/* new partitions (from other processors) */
	newPartitions = calloc(size, sizeof(intArray*));
	newPartitionsHead = calloc(1, sizeof(intArray));

	/* set local sizes */
	for (i = 0; i < size; i++) {
		partSizes->arr[i] = partitions[i]->size;
	}

	/* swap partition sizes */
	exchange_partition_sizes(rank, partSizes, newPartSizes);

	/* swap partitions */
	exchange_partitions(
	    rank, size,
	    partSizes, newPartSizes,
	    partitions, partitionsHead,
	    newPartitions, newPartitionsHead);

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
	free(partSizes->arr);
	free(partSizes);
	free(newPartSizes->arr);
	free(newPartSizes);
	return;
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

	/* do a k-way merge of already sorted lists */
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
	print_intArray(rank, finalList, "final list");
	printf("%d PHASE 4 DONE\n", rank);
#endif
	return;
}

/*
 * Phase 5 (not actually a phase in PSRS)
 *
 * Gather final lists from all processes
 * Concatenate final lists and pivots
 */
void phase_5(
    int rank, int size, int nElem, intArray *finalList, intArray *pivots) {

	int i = 0;
	int offset = 0;
	intArray *concatList = NULL;
	int sizes[size];
	int displs[size];
	
	memset(sizes, 0, size * sizeof(int));
	memset(displs, 0, size * sizeof(int));

	/* exchange finalList sizes with all processors */
	/* gather sizes at MASTER */
	MPI_Gather(
	    &(finalList->size),
	    1,
	    MPI_INT,
	    sizes,
	    1,
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);

	/* figure out displs for recieve */
	MASTER {
		offset = 0;
		for (i = 0; i < size; i++) {
			displs[i] = offset;
			/* +1 to add spaces between lists for pivots */
			offset += sizes[i] + 1;
		}
	}

	concatList = calloc(1, sizeof(intArray));
	/* all procs need concatList, but only master needs the arr memory */
	MASTER {
		concatList->size = nElem;
		concatList->arr = calloc(concatList->size, sizeof(int));
	}
	/* gather final lists on MASTER */
	MPI_Gatherv(
	    finalList->arr,
	    finalList->size,
	    MPI_INT,
	    concatList->arr,
	    sizes,
	    displs,
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);

	MASTER {
		/* insert pivots into concatList */
		offset = 0;
		for (i = 0; i < pivots->size; i++) {
			offset += sizes[i];
			concatList->arr[offset] = pivots->arr[i];
			offset++;
		}

#if DEBUG
		printf("%d PHASE 5 pivot insert complete\n", rank);
#endif
		/* print concatedated list for verification */
		for (i = 0; i < concatList->size; i++) {
			printf("%d, ", concatList->arr[i]);
		}
		printf("\n");

		free(concatList->arr);
	}

	free(concatList);

#if DEBUG
	printf("%d PHASE 5 DONE\n", rank);
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
