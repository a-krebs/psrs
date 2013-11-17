#ifndef TOD_H
#define TOD_H

#include <sys/time.h>


void print_TOD();
void print_elapsed_time(struct timeval*, struct timeval*);
int timeval_subtract(struct timeval*, struct timeval*, struct timeval*);

#endif
