#include <ucontext.h>
#include <stdio.h>    
#include <stdlib.h> 
#include <string.h>     
#include "dccthread.h"
#include "dlist.h"       

// Estruturas
typedef struct dccthread{
	ucontext_t context;
	char name[DCCTHREAD_MAX_NAME_SIZE];
    
    dccthread_t *thread_aguardada;
    int finalizada;

} dccthread_t;

dccthread_t *thread_gerente;
dccthread_t *principal;
dccthread_t *thread_atual;

struct dlist *threads_prontas;
struct dlist *threads_aguardando;

int total_threads = 0;

const char * dccthread_name(dccthread_t *tid){
	return tid->name;
}

dccthread_t * dccthread_self(void){
	return threads_prontas->head->data;
}

void set_initial_context(dccthread_t *thread, dccthread_t *link_thread) {
    if (link_thread != NULL) {
        thread->context.uc_link = &link_thread->context;
    }
    thread->finalizada = 0;
    thread->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
    thread->context.uc_stack.ss_size = THREAD_STACK_SIZE;
    thread->context.uc_stack.ss_flags = 0;
}

void dccthread_yield(void){
	dccthread_t *thread_atual = dccthread_self();

	dlist_push_right(threads_prontas, thread_atual);
	swapcontext(&thread_atual->context, &thread_gerente->context);
}

void dccthread_wait(dccthread_t *tid){
    if (tid->finalizada == 1) {
        return;
    }
	dccthread_t *thread_em_execucao = dccthread_self();
    thread_em_execucao->thread_aguardada = tid;
    dlist_push_right(threads_aguardando, thread_em_execucao);
    swapcontext(&thread_em_execucao->context, &thread_gerente->context);
}

int verifica_thread_aguardada(const void *t1, const void *t2, void *userdata){
    dccthread_t* thread_list = (dccthread_t*) t1;
	dccthread_t* thread_saindo = (dccthread_t*) t2;
	if(thread_saindo == thread_list->thread_aguardada){
		return 0;
    } else {
		return 1;
    }
}

void dccthread_exit(void){
	dccthread_t* thread_em_execucao = dccthread_self();
	dccthread_t* thread_aguardando = (dccthread_t*) dlist_find_remove(threads_aguardando, thread_em_execucao, verifica_thread_aguardada, NULL);
	if(thread_aguardando != NULL){
       // printf("%s Thread Aguardando\n", dccthread_name(thread_aguardando));
		dlist_push_right(threads_prontas, thread_aguardando);
	}
    thread_em_execucao->finalizada = 1;
	setcontext(&thread_gerente->context);
}

dccthread_t * dccthread_create(const char *name, void (*func)(int), int param) {
    dccthread_t *nova_thread;
	nova_thread = (dccthread_t*) malloc (sizeof(dccthread_t));
    getcontext(&nova_thread->context);
    strcpy(nova_thread->name, name);
    
    set_initial_context(nova_thread, thread_gerente);

    // Adiciona a thread a lista de threads prontas
    dlist_push_right(threads_prontas, nova_thread);
    makecontext(&nova_thread->context, (void *) func, 1, param);
    return nova_thread;
}

void gerenciador() {
    while(!dlist_empty(threads_prontas)) {
        dccthread_t *thread = (dccthread_t *) malloc (sizeof(dccthread_t));
        thread = threads_prontas->head->data;
        swapcontext(&thread_gerente->context, &thread->context);
        dlist_pop_left(threads_prontas);
    }
    exit(EXIT_SUCCESS);
}

void dccthread_init(void (*func)(int), int param) {
    // Inicializa as listas
    threads_prontas = dlist_create();
    threads_aguardando = dlist_create();

    thread_gerente = (dccthread_t *) malloc (sizeof(dccthread_t));
    getcontext(&thread_gerente->context);
    strcpy(thread_gerente->name, "gerente");

    set_initial_context(thread_gerente, NULL);
    makecontext(&thread_gerente->context, (void *) gerenciador, 0);

    principal = dccthread_create("main", func, param);

    setcontext(&thread_gerente->context);
   
    exit(EXIT_SUCCESS);
}