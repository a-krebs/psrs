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
	int size = -1;
	int rank = -1;
	double tStart = 0;
	double tEnd = 0;
	char hname[ 256 ];
	struct arguments args;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

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
	 * print
	 * rank, size, host, nElem, seed, time
	 */
	printf("%d, %d, %s, %d, %d, %f\n",
	    rank, size, hname, args.nElem, args.seed, tEnd- tStart);

	MPI_Finalize();

	exit(EXIT_SUCCESS);
}
