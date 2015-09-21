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

pthread_mutex_t thread_sum ;
// limit of numbers of thread 
static int limit = 4;
// current number of thread
int n_thread = 1;



static double sec(void)
{
	struct timeval t;
	gettimeofday(&t,NULL);
	return t.tv_sec + (t.tv_usec/1000000.0);
}

typedef int (*compare)(const void*, const void*);

typedef struct arg_struct_t {
	void* 		base; // listan
	size_t		n;	// Number of elements in base.
	size_t		s;	// Size of each element.
	compare    		cmp_func; // Behaves like strcmp
}arg_struct;

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

void* merge(double* left, double* right,size_t n_left,size_t n_right)
{
	double current_min;
	double*
	size_t left_index = 0;
	size_t right_index = 0;
	while( n_index<n_left && n_right<n_right ) {

		if(cmp(left,right)){
			swap(left,right)
		}

	}
}

void* par_sort( void* args )
{
	arg_struct arg1 = *(arg_struct*) args;
	void* base 	= 	arg1.base; 	// base pointer
	size_t n 	=	arg1.n;  	// number of elements in base
	size_t s 	= 	arg1.s; 	// size of each element
	compare cmp_func 	= arg1.cmp_func;

	int split 	= 0; //get from mutex
	pthread_mutex_lock (&thread_sum);
   	if(n_thread < limit){
   		split = 1;
   		n_thread+=1;
   	}
   	pthread_mutex_unlock (&thread_sum);
	 
	if(split){
		pthread_t thread;
		size_t middle = n/2;
		double pivot_element = base[n/2] 
		arg_struct arg0 = 
		{
			base,
			middle,
			s,
			cmp_func,
		};
		arg_struct arg1 = 
		{ 	
u7u7y			((char*) base) + middle , 
			n - middle,
			s ,
			cmp_func
		};

		if( pthread_create( &thread, NULL, par_sort,&arg1 ) ) {
			fprintf(stderr,"Error creating thread\n");
		}
		par_sort(&arg0);
		// end thread run with pthread_join
		if( pthread_join(thread,NULL) ){
			fprintf(stderr,"Error joining thread\n");
		}
		//merge result
	}else{
		qsort(base,n,s,cmp_func);
	}

	return NULL;
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
	
	pthread_mutex_init(&thread_sum, NULL);


	arg_struct arg = {a,n,sizeof a[0],cmp};
	par_sort(&arg);
#ifdef ASSERT
	qsort(b, n, sizeof a[0] , cmp);
#endif

	end = sec();


	printf("%1.2f s\n", (end - start)/CLOCKS_PER_SEC );
#ifdef ASSERT
	for(i = 0; i < n ; i++){
		assert(a[i] == b[i]);
	}
#endif
	free(a);
	pthread_mutex_destroy(&thread_sum);
	pthread_exit(NULL);
#ifdef ASSERT
	free(b);
#endif
	return 0;
}
