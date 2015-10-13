#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <omp.h>
#include "f2c.h"
#include "timebase.h"

double		begin;

static void print_score()
{
	uint64_t	score;
	FILE*		fp;
	double		end;

	end = timebase_sec();

	score = round((end-begin) * 1000);

	fp = fopen("score", "w");

	if (fp == NULL) {
		fprintf(stderr, "cannot open file \"score\" for writing");
		exit(1);
	}

	fprintf(fp, "%" PRIu64 "\n", score);
	fclose(fp);
}

void swim(void);

/* Main program alias */ int MAIN__ () 
{
	omp_set_num_threads(4);
	init_timebase();
	begin = timebase_sec();
	atexit(print_score);
	swim(); 
	return 0;
}
