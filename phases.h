/*
 * CMPUT 481 Assignment 2
 *
 * Aaron Krebs <akrebs@ualberta.ca>
 */

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
	/* "phase" 5 is the sorted-list-concatenation phase */
	double tPhase5S;	/* start of phase 5 */
	double tPhase5E;	/* end of phase 5 */
	double tEnd;
};

int run(struct timing*, int, int);
int *gen_rand_list(int);
