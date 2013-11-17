/*
 * CMPUT 481 Assignment 2
 *
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#ifndef ARGS_H
#define ARGS_H

/* struct for command line arguments */
struct arguments {
	/* size of input list */
	int nElem;
	/* seed value for generating random numbers */
	int seed;
};

int parse_args(int, char**, struct arguments*, int);
#endif
