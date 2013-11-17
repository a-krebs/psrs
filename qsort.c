/*
 * CMPUT 481 Assignment 2
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>	/* For gethostname() */
#include <string.h>	/* For memset() */

#include "args.h"
#include "tod.h"

int compare(const void *x, const void *y) {
	int a = *(int*)x;
	int b = *(int*)y;

	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

int main(int argc, char **argv)
{
	/* local variables */
	int size = 1;
	char hname[ 256 ];
	struct arguments args;
	/* locals for timing */
	struct timeval start;
	struct timeval end;
	/* list of data to sort */
	int *elem = NULL;
	int i = 0;

	/* parse command line arguments */
	memset((void*)&args, 0, sizeof(struct arguments));
	if (parse_args(argc, argv, &args, size) != 0) {
		exit(EXIT_FAILURE);
	}

	/* zero out times */
	memset((void*)&start, 0, sizeof(struct timeval));
	memset((void*)&end, 0, sizeof(struct timeval));

	elem = malloc(args.nElem * sizeof(int));

	/* seed random number genrator */
	srandom(args.seed);

	for (i = 0; i < args.nElem; i++) {
		elem[i] = (int)random();
	}

	/* get start time */
	gettimeofday(&start, NULL);

	/* sort elements */
	qsort(elem, args.nElem, sizeof(int), &compare);

	/* end time */
	gettimeofday(&end, NULL);\

	/* do output */
	memset((void*)hname, 0, 256);
	gethostname(hname, 255);
#if GATHERFINAL
	for (i = 0; i < args.nElem; i++) {
		printf("%d, ", elem[i]);
	}
	printf("\n");
#else
	/*
	 * CSV output
	 * alg, rank, size, host, nElem, seed,
	 * phase 1 time, phase 2 time, phase 3 time, phase 4 time,
	 * total time
	 */
	printf("seq,%d,%d,%s,%d,%d,-1,-1,-1,-1,",
	    0, 1, hname, args.nElem, args.seed);
	print_elapsed_time(&start, &end);
	printf("\n");
#endif

	/* clean up and exit */
	free(elem);
	exit(EXIT_SUCCESS);
}
