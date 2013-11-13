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

int main(int argc, char **argv)
{
	int size = -1;
	int rank = -1;
	double tStart = 0;
	double tEnd = 0;
	char hname[ 256 ];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	tStart = MPI_Wtime();
	
	memset((void*)hname, 0, 256);
	gethostname(hname, 255);

	MASTER {
		sleep(10);
	}

	tEnd = MPI_Wtime();

	printf("%d of %d running on %s took %f\n",
	    rank, size, hname, tEnd- tStart);

	MPI_Finalize();

	exit(EXIT_SUCCESS);
}
