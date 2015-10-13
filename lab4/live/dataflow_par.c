#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>
#include <pthread.h>
#include "dataflow.h"
#include "error.h"
#include "list.h"
#include "set.h"
#include <unistd.h>
#define NBR_THREADS (16)

typedef struct vertex_t	vertex_t;
typedef struct task_t	task_t;
typedef struct arg_struct_t arg_struct_t;
//typedef struct arena_t        arena_t;
typedef struct arena_list_t   arena_list_t;

/*struct arena_t{
    size_t offset;
    size_t size;
    void* allocated;
};

static arena_t* arena = NULL;


static arena_t* create_arena(size_t size)
{
	arena_t* arena = (arena_t*)malloc(sizeof(arena_t));
	arena->allocated=malloc(size);
	arena->size=size;
	arena->offset=0;
	return arena;
}

static void free_arena(arena_t* arena)
{
	free(arena->allocated);
	free(arena);
}

static void* amalloc(arena_t* arena, size_t size)
{
	size_t would_be_allocated=arena->offset+size;
	if(would_be_allocated>arena->size){
		return NULL;
	}
	void* space=arena->allocated+arena->offset;
	arena->offset=would_be_allocated;
  //printf("offset %zu\n", arena -> offset);
	return space;
}*/



struct arena_list_t{
    arena_list_t* pred;
    arena_list_t* succ;
    void* data;
};

static arena_list_t* free_head = NULL;

void take_out(arena_list_t* list)
{
	list->pred->succ = list->succ;
	list->succ->pred = list->pred;
	list->succ = list->pred = list;
}

int empty(arena_list_t* list){
	return list == list->succ;
}

arena_list_t* anew_list(void *data)
{

  /*if(arena == NULL){
  // printf("should make new arena\n");
    arena = create_arena(4096*10000);
  }*/
	arena_list_t*		list;
  //printf("should malloc arena\n");
	//list = amalloc(arena,sizeof(arena_list_t));
	if(free_head == NULL || empty(free_head)){
		list = malloc(sizeof(arena_list_t));
	}else{
		list = free_head->succ;
		take_out(list);
	}
	if (list == NULL) {
		fprintf(stderr, "out of memory in %s\n", __func__);
		exit(1);
	}

	list->succ = list->pred = list;
	list->data = data;

	return list;
}

void ainsert_last(arena_list_t** list1, void *p)
{
	arena_list_t*		tmp;
	arena_list_t*		list2;

	list2 = anew_list(p);

	if (*list1 == NULL)
		*list1 = list2;
	else if (list2 != NULL) {
		(*list1)->pred->succ = list2;
		list2->pred->succ = *list1;
		tmp	= (*list1)->pred;
		(*list1)->pred = list2->pred;
		list2->pred = tmp;
	}
}

void aappend(arena_list_t** list1, arena_list_t* list2)
{
	arena_list_t*		tmp;

	if (*list1 == NULL)
		*list1 = list2;
	else if (list2 != NULL) {
		(*list1)->pred->succ = list2;
		list2->pred->succ = *list1;
		tmp	= (*list1)->pred;
		(*list1)->pred = list2->pred;
		list2->pred = tmp;
	}
}

void adelete_list(arena_list_t *list)
{
	list->pred->succ = list->succ;
	list->succ->pred = list->pred;

	list->succ = free_head;
	list->pred = free_head->pred;
	free_head-> pred->succ = list;

}

void ainsert_before(arena_list_t** list, void *data)
{
	arena_list_t*		p;

	p = anew_list(data);

	if (*list == NULL)
		*list = p;
	else
		aappend(&p, *list);
}

void ainsert_after(arena_list_t** list, void *data)
{
	arena_list_t*		p;
	arena_list_t*		q;

	p = anew_list(data);

	if (*list == NULL)
		*list = p;
	else {
		q = (*list)->succ;
		aappend(&q, p);
	}
}

size_t alength(arena_list_t *list)
{
	arena_list_t*		p;
	size_t		n;

	if (list == NULL)
		return 0;
	else {
		p = list;
		n = 0;
		do {
			++n;
			p = p->succ;
		} while (p != list);
		return n;
	}
}

void* aremove_first(arena_list_t** list)
{
	void*		data;
	arena_list_t*		p;

	if (*list == NULL)
		return NULL;
	p = *list;
	data = p->data;

	if (*list == (*list)->succ){
		*list = NULL;
	}
	else
		*list = p->succ;
	adelete_list(p);
	return data;
}

void afree_list(arena_list_t** list)
{
	arena_list_t*		p;
	arena_list_t*		q;

	p = *list;

	if (p == NULL)
		return;

	p->pred->succ = NULL;

	while (p != NULL) {
		q = p;
		p = p->succ;
		free(q);
	}
	if(free_head != NULL)
		afree_list(&free_head);
  //free_arena(arena);
	*list = NULL;
}

pthread_mutex_t work_mutex;

struct arg_struct_t{
	//enter arguments
	arena_list_t** worklist;
	size_t			size;

};


/* cfg_t: a control flow graph. */
struct cfg_t {
	size_t			nvertex;	/* number of vertices		*/
	size_t			nsymbol;	/* width of bitvectors		*/
	vertex_t*		vertex;		/* array of vertex		*/
};

/* vertex_t: a control flow graph vertex. */
struct vertex_t {
	size_t			index;		/* can be used for debugging	*/
	set_t*			set[NSETS];	/* live in from this vertex	*/
	set_t*			prev;		/* alternating with set[IN]	*/
	size_t			nsucc;		/* number of successor vertices */
	vertex_t**		succ;		/* successor vertices 		*/
	list_t*			pred;		/* predecessor vertices		*/
	bool			listed;		/* on worklist			*/
  pthread_mutex_t mutex;
};

static void clean_vertex(vertex_t* v);
static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ);

cfg_t* new_cfg(size_t nvertex, size_t nsymbol, size_t max_succ)
{
	size_t		i;
	cfg_t*		cfg;

	cfg = calloc(1, sizeof(cfg_t));
	if (cfg == NULL)
		error("out of memory");

	cfg->nvertex = nvertex;
	cfg->nsymbol = nsymbol;

	cfg->vertex = calloc(nvertex, sizeof(vertex_t));
	if (cfg->vertex == NULL)
		error("out of memory");

	for (i = 0; i < nvertex; i += 1)
		init_vertex(&cfg->vertex[i], i, nsymbol, max_succ);

	return cfg;
}

static void clean_vertex(vertex_t* v)
{
	int		i;

	for (i = 0; i < NSETS; i += 1)
		free_set(v->set[i]);
	free_set(v->prev);
	free(v->succ);
	free_list(&v->pred);
}

static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ)
{
	int		i;

	v->index	= index;
	v->succ		= calloc(max_succ, sizeof(vertex_t*));

	if (v->succ == NULL)
		error("out of memory");

	for (i = 0; i < NSETS; i += 1)
		v->set[i] = new_set(nsymbol);

	v->prev = new_set(nsymbol);
}

void free_cfg(cfg_t* cfg)
{
	size_t		i;

	for (i = 0; i < cfg->nvertex; i += 1)
		clean_vertex(&cfg->vertex[i]);
	free(cfg->vertex);
	free(cfg);
}

void connect(cfg_t* cfg, size_t pred, size_t succ)
{
	vertex_t*	u;
	vertex_t*	v;

	u = &cfg->vertex[pred];
	v = &cfg->vertex[succ];

	u->succ[u->nsucc++ ] = v;
	insert_last(&v->pred, u);
}

bool testbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	return test(cfg->vertex[v].set[type], index);
}

void setbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	set(cfg->vertex[v].set[type], index);
}


void thread_liveness(void *args)
{
	arg_struct_t* t = (arg_struct_t*)args;
	arena_list_t** worklist = t->worklist;
	size_t		id = t->size;
	set_t* prev;
	vertex_t* u;
	size_t		j;
	list_t*		p;
	list_t*		h;
	vertex_t*	v;
	size_t 		count = 0;
	size_t		sum = 0;
	//printf("this shoudl be zero workload %zu, id %zu\n", count,id);

	do{
		pthread_mutex_lock(&work_mutex);
		u = aremove_first(worklist);
		pthread_mutex_unlock(&work_mutex);
		count++;
		if(u ==NULL){
			printf("workload %zu, id %zu\n", count,id);
			return;
		}
		u->listed = false;
		reset(u->set[OUT]);

		//pthread_mutex_lock(&work_mutex);
		for( j = 0; j< u->nsucc; ++j){
      pthread_mutex_lock(&u->succ[j]->mutex);
			or(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);
      pthread_mutex_unlock(&u->succ[j]->mutex);
    }


		/* in our case liveness information... */
    pthread_mutex_lock(&u->mutex);
		prev = u->prev;
		u->prev = u->set[IN];
		u->set[IN] = prev;
		propagate(u->set[IN], u->set[OUT], u->set[DEF], u->set[USE]);
    pthread_mutex_unlock(&u->mutex);

		if (u->pred != NULL && !equal(u->prev, u->set[IN])) {
			p = h = u->pred;
			do {
				v = p->data;
				if (!v->listed) {
					v->listed = true;
					pthread_mutex_lock(&work_mutex);
					ainsert_last(worklist, v);
					pthread_mutex_unlock(&work_mutex);
				}

				p = p->succ;

			} while (p != h);
		//pthread_mutex_unlock(&work_mutex);
		}
	}while(u != NULL);


}

void liveness(cfg_t* cfg)
{

	vertex_t*	u;
	vertex_t* n;
	size_t		i;
	free_head = anew_list(NULL);
  pthread_mutex_init ( &work_mutex, NULL);
	arena_list_t*		worklist = NULL;
	//*worklist = NULL;



	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i];
    pthread_mutex_init ( &u->mutex, NULL);
		//printf("doing upper stuff %zu\n",i);
		ainsert_last(&worklist, u);


		u->listed = true;
	}
	/*struct timeval	tv;

	gettimeofday(&tv, NULL);

	double end =tv.tv_sec + 1e-6 * tv.tv_usec;
	printf("T = %8.4lf s\n\n", end-begin);*/
	arg_struct_t args1[NBR_THREADS];
	pthread_t thread[NBR_THREADS];

	for(i = 0; i<NBR_THREADS; ++i){
		args1[i].worklist = &worklist;
		args1[i].size = i;

		pthread_create(&thread[i], NULL, &thread_liveness, &args1[i]);
	}


	for(i = 0; i<NBR_THREADS; ++i)
		pthread_join(thread[i], NULL);

}

void print_sets(cfg_t* cfg, FILE *fp)
{
	size_t		i;
	vertex_t*	u;

	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i];
		fprintf(fp, "use[%zu] = ", u->index);
		print_set(u->set[USE], fp);
		fprintf(fp, "def[%zu] = ", u->index);
		print_set(u->set[DEF], fp);
		fputc('\n', fp);
		fprintf(fp, "in[%zu] = ", u->index);
		print_set(u->set[IN], fp);
		fprintf(fp, "out[%zu] = ", u->index);
		print_set(u->set[OUT], fp);
		fputc('\n', fp);
	}
}
