/*
 * CMPUT 481 Assignment 2
 *
 * Aaron Krebs <akrebs@ualberta.ca>
 */

#include "args.h"	/* for struct arguments */

/* struct for timing values */
struct timing {
	double tStart;
	double tPhase1S;	/* start of phase 1 */
	double tPhase1E;	/* end of phase 1 */
	double tPhase2S;	/* start of phase 2 */
	double tPhase2E;	/* end of phase 2 */
	double tPhase3S;	/* start of phase 3 */
	double tPhase3E;	/* end of phase 3 */
	double tPhase4S;	/* start of phase 4 */
	double tPhase4E;	/* end of phase 4 */
	double tEnd;
};

typedef struct {
	int size;
	int *arr;
} intArray;

int run(struct timing*, struct arguments*, int, int);
void print_intArray(int, intArray*, char*);
int *gen_rand_list(int, int);
void scatter_data(int*, int*, int);
void phase_1(int, intArray*, intArray*, intArray*, int);
void gather_samples(int, int, intArray*, intArray*);
void broadcast_pivots(int, int, intArray*, intArray*);
void reset_arr_pointers(intArray*, intArray**, int);
void partition_data(int, int, intArray*, intArray*, intArray**, intArray*);
void phase_2(int, int , intArray*, intArray*, intArray*, intArray**, intArray*);
void exchange_partition_sizes(int, intArray*, intArray*);
void exchange_partitions(int, int, intArray*, intArray*,
    intArray**, intArray*, intArray**, intArray*);
void phase_3(int, int, intArray***, intArray**);
void phase_4(int, int, intArray**, intArray*);
void phase_5(int, int, int, intArray*, intArray*);
int compare(const void*, const void*);
