/*
 * CMPUT 481 Assignment 2
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>	/* For gethostname() */
#include <string.h>	/* For memset() */
#include "mpi.h"

#include "macros.h"
#include "args.h"
#include "phases.h"

int main(int argc, char **argv)
{
	/* local variables */
	int size = -1;
	int rank = -1;
	char hname[ 256 ];
	struct arguments args;
	/* locals for timing */
	struct timing times;

	/* initialize MPI and get size and rank */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* parse command line arguments */
	memset((void*)&args, 0, sizeof(struct arguments));
	if (parse_args(argc, argv, &args, size) != 0) {
		MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	/* zero out times */
	memset((void*)&times, 0, sizeof(struct timing));

	/* run PSRS */
	if (run(&times, &args, rank, size) != 0) {
		MPI_Finalize();
		exit(EXIT_FAILURE);
	}

	memset((void*)hname, 0, 256);
	gethostname(hname, 255);
#if !GATHERFINAL
	/*
	 * CSV output
	 * rank, size, host, nElem, seed,
	 * phase 1 time, phase 2 time, phase 3 time, phase 4 time,
	 * phase 5 (sorted list concatenation phase) time, total time
	 */
	MASTER {
		printf("psrs,%d,%d,%s,%d,%d,%f,%f,%f,%f,%f\n",
		    rank, size, hname, args.nElem, args.seed,
		    times.tPhase1E - times.tPhase1S,
		    times.tPhase2E - times.tPhase2S,
		    times.tPhase3E - times.tPhase3S,
		    times.tPhase4E - times.tPhase4S,
		    times.tEnd - times.tStart);
	}
#endif

	/* clean up and exit */
#if DEBUG
	printf("%d EXIT_SUCCESS\n", rank);
#endif
	MPI_Finalize();
	exit(EXIT_SUCCESS);
}
