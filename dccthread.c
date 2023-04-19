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

} dccthread_t;

dccthread_t *gerente;
dccthread_t *principal;

struct dlist *threads_prontas;
struct dlist *threads_aguardando;

void t1(int param) {
    printf("t1: %d", param);
    exit(EXIT_SUCCESS);
}

const char * dccthread_name(dccthread_t *tid){
	return tid->name;
}

dccthread_t * dccthread_create(const char *name, void (*func)(int), int param) {
    dccthread_t *nova_thread;
	nova_thread = (dccthread_t*) malloc (sizeof(dccthread_t));
    getcontext(&nova_thread->context);

    // Adiciona a thread a lista de threads prontas
    dlist_push_right(threads_prontas, nova_thread);
    makecontext(&nova_thread->context, (void (*)(void)) func, 1, param);
    return nova_thread;
}

void dccthread_init(void (*func)(int), int param) {
    // Inicializa as listas
    threads_prontas = dlist_create();
    threads_aguardando = dlist_create();

    gerente = (dccthread_t *) malloc (sizeof(dccthread_t));
    getcontext(&gerente->context);
    strcpy(gerente->name, "gerente");

    principal = dccthread_create("main", func, param);
    while(!dlist_empty(threads_prontas)) {
        dccthread_t *thread = dlist_pop_left(threads_prontas);
        swapcontext(&gerente->context, &thread->context);
    }
    exit(0);
}
void main() {
    dccthread_init(t1, 1);
}