/*
 * CMPUT 481 Assignment 2
 *
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#include <stdio.h>
#include <stdlib.h>	/* atoi() */
#include <unistd.h>	/* getopt() */
#include <ctype.h>	/* isprint() */

#include "args.h"

/*
 * Parse arguments into given struct.
 * Struct memory area should be initialized.
 */
int parse_args(int argc, char *argv[], struct arguments* args, int procs) {
	/* expected arguments */
	int nElem = 0;
	unsigned int seed = 1000;	// default;

	/* vars needed for parsing */
	int index;
	int option;

	opterr = 0;
	while ((option = getopt(argc, argv, "n:s:")) != -1) {
		switch (option) {
		case 'n':
			nElem = atoi(optarg);
			break;
		case 's':
			seed = atoi(optarg);
			break;
		case '?':
			if ((optopt == 'n')
			    || (optopt == 's')){
				fprintf(
				    stderr,
				    "Option -%c requires an argument.\n",
				    optopt);
			} else if (isprint(optopt)) {
				fprintf(
				    stderr,
				    "Unknown option `-%c'.\n",
				    optopt);
			} else {
				fprintf(
				    stderr,
				    "Unknown option character `\\x%x'.\n",
				    optopt);
			}
			/* fall through to default */
		default:
			return -1;
		}
	}
	
#if DEBUG
	printf("n = %d, s = %d\n", nElem, seed);
#endif

	/* Make sure p is at least 1 */
	if (procs < 1) {
		fprintf(stderr, "p must be at least 1.\n");
		return -1;
	}

	/* Make sure n is evenly divisible into p */
	if ((nElem > 0) && (nElem % procs != 0)) {
		fprintf(stderr, "n must be evenly divisible into p\n");
		return -1;
	}
	for (index = optind; index < argc; index++) {
		fprintf(stderr, "Non-option argument %s\n", argv[index]);
		return -1;
	}

	args->nElem = nElem;
	args->seed = seed;
	return 0;
}
