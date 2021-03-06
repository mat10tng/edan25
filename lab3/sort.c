#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>

#define ASSERT

typedef struct arg_struct_t {
	void* 		base; // listan
	size_t		n;	// Number of elements in base.
	size_t		s;	// Size of each element.
	int		(*cmp)(const void*, const void*); // Behaves like strcmp
	int 		once;
}arg_struct;

static double sec(void)
{
	return clock();
}

void print(double* a, size_t size)
{
	printf("-----------------\n");
	size_t i;
	for(i = 0; i < size; i++)
		printf("%G\n",a[i]);
}

void swap(double* left, double* right)
{
	double temp = *left;
	*left = *right;
	*right = temp; 
}

void insertion_sort(double* list, size_t n)
{

} 
//not use
void merge_sort_rec(double* list, size_t n)
{
	if( n <= 6){
		insertion_sort(list,n);
		return;
	}
	size_t middle = n/2;
	size_t left = 0;
	size_t right = n - n/2;
	merge_sort_rec(list, middle);
	merge_sort_rec(&list[right], (n - middle));
	// beware the range. It might kill you
	size_t count = 0;
}


void par_sort(
	void*		base,	// Array to sort.
	size_t		n,	// Number of elements in base.
	size_t		s,	// Size of each element.
	int		(*cmp)(const void*, const void*), // Behaves like strcmp
	int 		once 						)
{
	if(once == 1){
		pthread_t thread;
		// maybe need some counter for thread
		arg_struct arg = {base,n,s,cmp,0};
		if( pthread_create( &thread, NULL, par_sort,&arg ) ) {
			fprintf(stderr,"Error creating thread\n");
			exit;
		}

		// end thread run with pthread_join
		if( pthread_join(thread,NULL) ){
			fprintf(stderr,"Error joining thread\n");
			exit;
		}

	}else{
		printf("anytthing\n");
		print(base,n);
		qsort(base, n, s , cmp);
		print(base,n);
		printf("thing should change here\n");
	}
}
int cmp_double(double a, double b){
	return a < b ? -1 : a == b ? 0 : 1; 
}
static int cmp(const void* ap, const void* bp)
{	
	/* you need to modify this function to compare doubles. */
	double a;
	double b;
	a = * (const double*) ap;
	b = * (const double*) bp;
	return cmp_double(a,b);
}


int main(int ac, char** av)
{
	int		n = 2000000;
	n = 5;
	int		i;
	double*		a;
#ifdef ASSERT
	double* 	b;
#endif
	double		start, end;

	if (ac > 1)
		sscanf(av[1], "%d", &n);

	srand(getpid());
	a = malloc(n * sizeof a[0]);
#ifdef ASSERT
	b = malloc(n * sizeof a[0]);
#endif
	for (i = 0; i < n; i++){
		a[i] = rand();
#ifdef ASSERT
		b[i] = a[i];
#endif
	}
	start = sec();
	
	par_sort(a, n, sizeof a[0], cmp,1);
#ifdef ASSERT
	qsort(b, n, sizeof a[0] , cmp);
#endif

	end = sec();

	print(a,n);
	print(b,n);

	printf("%1.2f s\n", (end - start)/CLOCKS_PER_SEC );
#ifdef ASSERT
	for(i = 0; i < n ; i++){
		assert(a[i] == b[i]);
	}
#endif
	free(a);
#ifdef ASSERT
	free(b);
#endif
	return 0;
}
