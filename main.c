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

int main(int argc, char **argv)
{
	/* local variables */
	int size = -1;
	int rank = -1;
	char hname[ 256 ];
	struct arguments args;
	/* locals for timing */
	double tStart = 0;
	double tPhase1S = 0;	/* start of phase 1 */
	double tPhase1E = 0;	/* end of phase 1 */
	double tPhase2S = 0;	/* start of phase 2 */
	double tPhase2E = 0;	/* end of phase 2 */
	double tPhase3S = 0;	/* start of phase 3 */
	double tPhase3E = 0;	/* end of phase 3 */
	double tPhase4S = 0;	/* start of phase 4 */
	double tPhase4E = 0;	/* end of phase 4 */
	/* "phase" 5 is the sorted-list-concatenation phase */
	double tPhase5S = 0;	/* start of phase 5 */
	double tPhase5E = 0;	/* end of phase 5 */
	double tEnd = 0;

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

	tStart = MPI_Wtime();
	
	memset((void*)hname, 0, 256);
	gethostname(hname, 255);

	MASTER {
		sleep(3);
	}

	tEnd = MPI_Wtime();

	/*
	 * CSV output
	 * rank, size, host, nElem, seed,
	 * phase 1 time, phase 2 time, phase 3 time, phase 4 time,
	 * phase 5 (sorted list concatenation phase) time, total time
	 */
	printf("%d, %d, %s, %d, %d, %f, %f, %f, %f, %f, %f\n",
	    rank, size, hname, args.nElem, args.seed,
	    tPhase1E - tPhase1S,
	    tPhase2E - tPhase2S,
	    tPhase3E - tPhase3S,
	    tPhase4E - tPhase4S,
	    tPhase5E - tPhase5S,
	    tEnd - tStart);


	/* clean up and exit */
	MPI_Finalize();
	exit(EXIT_SUCCESS);
}
