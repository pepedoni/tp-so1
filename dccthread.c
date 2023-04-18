#include <stdio.h>                                                                                                                                                                                                                                                                                                                                           #include "dccthread.h"
#include <ucontext.h>

#include "dccthread.h"
#include "dlist.h"

// Estruturas
typedef struct dccthread{
	ucontext_t context;
	char name[DCCTHREAD_MAX_NAME_SIZE];

} dccthread_t;

dccthread_t *gerente;
dccthread_t *principal;

dccthread_t * dccthread_create(const char *name, void (*func)(int), int param) {
 
    
}

void dccthread_init(void (*func)(int), int param) {
    gerente = (dccthread_t *) malloc (sizeof(dccthread_t));
    getcontext(&gerente->context);

    principal = (dccthread_t *) malloc (sizeof(dccthread_t));
    getcontext(&principal->context);

}

void t1(int param) {
    printf("t1: %d", param);
}

const char * dccthread_name(dccthread_t *tid){
	return tid->name;
}


void main() {
    dccthread_init(t1, 2);
}