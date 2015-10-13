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


#define NTHREADS (4)
#define MINWORK (6)
#define MAXWORK (2500)
pthread_mutex_t work_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;
void lock()
{
        pthread_mutex_lock(&work_lock);
}
void unlock()
{
        pthread_mutex_unlock(&work_lock);
}
void notifyAll()
{
        pthread_cond_broadcast(&cond);
}
void wait()
{
        pthread_cond_wait(&cond, &work_lock);
}
typedef struct vertex_t        vertex_t;
typedef struct task_t        task_t;

/* cfg_t: a control flow graph. */
struct cfg_t {
        size_t nvertex;        /* number of vertices                */
        size_t nsymbol;        /* width of bitvectors                */
        vertex_t* vertex;                /* array of vertex                */
};

/* vertex_t: a control flow graph vertex. */
struct vertex_t {
        size_t index;                /* can be used for debugging        */
        set_t* set[NSETS];        /* live in from this vertex        */
        set_t* prev;                /* alternating with set[IN]        */
        size_t nsucc;                /* number of successor vertices */
        vertex_t** succ;                /* successor vertices                 */
        list_t* pred;                /* predecessor vertices                */
        bool listed;                /* on worklist                        */
};


static void clean_vertex(vertex_t* v);
static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ);
void node_liveness(vertex_t* u, list_t** new_work);

cfg_t* new_cfg(size_t nvertex, size_t nsymbol, size_t max_succ)
{
        size_t                i;
        cfg_t*                cfg;

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
        int                i;

        for (i = 0; i < NSETS; i += 1)
                free_set(v->set[i]);
        free_set(v->prev);
        free(v->succ);
        free_list(&v->pred);
}

static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ)
{
        int                i;

        v->index        = index;
        v->succ                = calloc(max_succ, sizeof(vertex_t*));

        if (v->succ == NULL)
                error("out of memory");

        for (i = 0; i < NSETS; i += 1)
                v->set[i] = new_set(nsymbol);

        v->prev = new_set(nsymbol);
}

void free_cfg(cfg_t* cfg)
{
        size_t                i;

        for (i = 0; i < cfg->nvertex; i += 1)
                clean_vertex(&cfg->vertex[i]);
        free(cfg->vertex);
        free(cfg);
}

void connect(cfg_t* cfg, size_t pred, size_t succ)
{
        vertex_t*        u;
        vertex_t*        v;

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

static int n_rem_work = 0;
static list_t* remaining_work = NULL;
static int n_waiting = 0;
size_t nsymbols;
/**
 * Returns the number of work in the list
 */
int work_is_finished()
{
    return !n_rem_work && NTHREADS == n_waiting;
}

void add_work(list_t* new_work)
{
        n_rem_work += length(new_work);

        append(&remaining_work, new_work);
        notifyAll();
}


list_t* look_for_work()
{
    list_t* work = NULL;
    while(!work_is_finished() && !n_rem_work){
        wait();
    }
    if(n_rem_work){
        //make sure to take the right amount of work
        int n_work = n_rem_work / NTHREADS + 1;
        n_work = n_work < MINWORK ? MINWORK : n_work;
        n_work = n_work > MAXWORK ? MAXWORK : n_work;
        n_work = n_work > n_rem_work ? n_rem_work : n_work;
        n_rem_work -= n_work;
        //retrieve the work
        for(int i=0;i< n_work; ++i){
                vertex_t* data = remove_first(&remaining_work);
                insert_last(&work, data);
        }
    }
    return work;
}

list_t* get_work(list_t* new_work)
{
        lock();
        n_waiting++;
        add_work(new_work);
        list_t* work = look_for_work();
        if(work != NULL)
            n_waiting--;
        unlock();
        return work;
}

void* thread_work(void* dummy_arg)
{
    size_t mywork = 0;
    list_t* new_work = NULL;
    list_t* work = get_work(new_work);
    vertex_t* vertex;
    while(work!=NULL){
        vertex = remove_first(&work);
        while(vertex!=NULL){
            mywork++;
            node_liveness(vertex, &new_work);
            vertex = remove_first(&work);
        }
        work = get_work(new_work);
        new_work = NULL;
    }
    printf("mywork was: %zu\n", mywork);
    return NULL;
}


void node_liveness(vertex_t* u, list_t** new_work)
{
        vertex_t*        v;
        set_t*                prev;
        size_t                j;
        list_t*                p;
        list_t*                h;

        u->listed = false;

        reset(u->set[OUT]);

        for (j = 0; j < u->nsucc; ++j){
                or(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);
        }
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
                                insert_last(new_work, v);
                        }

                        p = p->succ;

                } while (p != h);
        }

}


void liveness(cfg_t* cfg)
{
        printf("doing live\n");
        vertex_t*        u;
        size_t                i;

        remaining_work = NULL;
        for (i = 0; i < cfg->nvertex; ++i) {
                n_rem_work++;
                u = &cfg->vertex[i];

                insert_last(&remaining_work, u);
                u->listed = true;
        }
        nsymbols = cfg->nsymbol;

        pthread_t threads[NTHREADS];

        for(int i=0;i<NTHREADS; ++i){
            pthread_create(&threads[i], NULL, thread_work, NULL);
        }

        //wait for threads
        for(int i=0; i<NTHREADS; ++i){
                if(pthread_join(threads[i], NULL)) {
                    fprintf(stderr, "Error joining thread\n");
                }
        }

}

void print_sets(cfg_t* cfg, FILE *fp)
{
        size_t                i;
        vertex_t*        u;

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
