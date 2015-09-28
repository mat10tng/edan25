#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "dataflow.h"
#include "error.h"
#include "list.h"
#include "set.h"

typedef struct vertex_t	vertex_t;
typedef struct task_t	task_t;
pthread_mutex_t worklist_mutex;

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
};
static void clean_vertex(vertex_t* v);
static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ);
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
void compute(vertex_t* u,list_t* worklist)
{
	vertex_t* v;
	size_t j;
	set_t* prev;
	list_t* p;
	list_t* h;
		
	u->listed = false;

	reset(u->set[OUT]);

	for (j = 0; j < u->nsucc; ++j)
		or(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);

	prev = u->prev;
	u->prev = u->set[IN];
	u->set[IN] = prev;

	/* in our case liveness information... */
	propagate(u->set[IN], u->set[OUT], u->set[DEF], u->set[USE]);

	if (u->pred != NULL && !equal(u->prev, u->set[IN])) {
		p = h = u->pred;
		do {
			v = p->data;
			if (!v->listed) {
				v->listed = true;
				pthread_mutex_lock(&worklist_mutex);
				insert_last(&worklist, v);
				pthread_mutex_unlock(&worklist_mutex);
			}

			p = p->succ;

		} while (p != h);
	}
}
void* work(void* wlist)
{
	list_t* worklist = wlist;
	vertex_t* u;

	pthread_mutex_lock(&worklist_mutex);
	u = remove_first(&worklist);
	pthread_mutex_unlock(&worklist_mutex);
	while(u!= NULL){
		compute(u,worklist);
		pthread_mutex_lock(&worklist_mutex);
		u = remove_first(&worklist);
		pthread_mutex_unlock(&worklist_mutex);
	}
	return NULL;
}
void works_dealer(list_t* worklist)
{
	size_t thread_limit = 1;
	pthread_t workers[thread_limit];

	size_t t;
	for( t = 0; t < thread_limit; ++t){
		if ( pthread_create( &workers[t],NULL, work,worklist)){
			fprintf(stderr,"Error creating thread\n");
		}
	}
	for( t = 0; t < thread_limit; ++t){
		if ( pthread_join( workers[t],NULL)){
			fprintf(stderr,"Error joining thread\n");
		}
	}

}
void liveness(cfg_t* cfg)
{
	vertex_t*	u;
	size_t		i;
	list_t*		worklist;
	worklist = NULL;	
	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i];
		insert_last(&worklist, u);
		u->listed = true;
	}
	
	pthread_mutex_init(&worklist_mutex,NULL);
	works_dealer(worklist);	
	pthread_mutex_destroy(&worklist_mutex);
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
