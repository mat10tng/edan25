#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>

// #define PARALLEL

static double sec(void)
{
	return clock();
}

void par_sort(
	void*		base,	// Array to sort.
	size_t		n,	// Number of elements in base.
	size_t		s,	// Size of each element.
	int		(*cmp)(const void*, const void*)) // Behaves like strcmp
{


}

static int cmp(const void* ap, const void* bp)
{	
	/* you need to modify this function to compare doubles. */
	double a;
	double b;
	a = *(const double*) ap;
	b = *(const double*) bp;
	return a < b ? -1 : a == b ? 0 : 1; 
}
void print(double* a, size_t size)
{
	printf("----------begin-------\n");
	size_t i;
	for(i = 0; i < size; i++)
		printf("%G\n",a[i]);
	printf("----------end---------\n");
}
int main(int ac, char** av)
{
	int		n = 2000000;
	int		i;
	double*		a;
	double		start, end;

	if (ac > 1)
		sscanf(av[1], "%d", &n);

	srand(getpid());

	a = malloc(n * sizeof a[0]);
	for (i = 0; i < n; i++)
		a[i] = rand();

	start = sec();
	


#ifdef PARALLEL
	par_sort(a, n, sizeof a[0], cmp);
#else
	qsort(a, n, sizeof a[0] , cmp);
#endif

	end = sec();

	printf("%1.2f s\n", (end - start)/CLOCKS_PER_SEC );

	free(a);

	return 0;
}
