/*
 * CMPUT 481 Assignment 2
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#include <stdio.h>
#include <unistd.h>	/* For gethostname() */
#include <string.h>	/* For memset() */
#include "mpi.h"

int main(int argc, char **argv)
{
	int size, rank;
	char hname[ 256 ];

	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	memset( (void*)hname, 0, 256 );
	gethostname( hname, 255 );

	printf( "%d of %d running on %s\n", rank, size, hname );

	MPI_Finalize();
}
