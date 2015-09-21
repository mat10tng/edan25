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
typedef int (*compare)(const void*, const void*);

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

int cmp_double(double a, double b){
	int result = a < b ? -1 : a == b ? 0 : 1;
//	printf("%1.2f %d %1.2f\n",a,result,b);
	return result; 
}
static int cmp(const void* ap, const void* bp)
{	
	/* you need to modify this function to compare doubles. */
	double a;
	double b;
	a = * (const double*) ap;
	b = * (const double*) bp;
	int result = cmp_double(a,b);
	return cmp_double(a,b);
}


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
	for(i = 0; i < size; i++){
		printf("%G\n",a[i]);
	}
	printf("---------------------\n");
}

void swap(double* left, double* right)
{
	double temp = *left;
	*left = *right;
	*right = temp; 
}

int partition (double* base, size_t size)
{	
	size_t i;
	double pivot;
	size_t j;
	i = 0;
	if(size > 1){
		pivot = base[size-1];
		for (j = 0; j < size-1; ++j){
			if (base[j] <= pivot) {
//				printf("%zu %zu \n", i , j);
				swap(&base[i],&base[j]);
				i += 1;
			}
		}
		swap(&base[i],&base[size-1]);
	}
	return i;
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
	 
	if(split && n > 5){
		pthread_t thread;
		int middle = partition(base,n);
		if(middle == 0){
			arg_struct arg0 = { ((double*) base) + 1, n-1, s, cmp_func};
			par_sort(&arg0);
			return;
		}
		printf("middle index is %zu of %zu ,percentage %1.2f\n", middle,n,middle*1.0 /n);
		arg_struct arg0 = { base, middle, s, cmp_func};
		arg_struct arg1 = {	((double*) base) + middle + 1 , n - middle - 1 , s, cmp_func };
		
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

//	print(a,n);
	printf("---before par sort---");
	par_sort(&arg);
	printf("---after par sort---");
#ifdef ASSERT
	qsort(b, n, sizeof a[0] , cmp);
#endif

	end = sec();


	printf("%1.2f s\n", (end - start) );
#ifdef ASSERT
	for(i = 0; i < n ; i++){
		if(a[i] != b[i]){
			printf("%d \n %1.2f  ",i, a[i]);
			printf("%1.2f  \n",b[i]);
		}
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
