#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include "dccthread.h"
#include "dlist.h"       

// Estruturas
typedef struct dccthread{
	ucontext_t context;
	char name[DCCTHREAD_MAX_NAME_SIZE];
    
    // Thread a qual essa thread precisa aguardar para finalizar
    dccthread_t *thread_aguardada;
    // Thread já foi finalizada ( 0 - Não, 1 - Sim )
    int finalizada;
    // Thread é aguardada por outra thread ( 0 - Não, 1 - Sim )
    int aguardada;

} dccthread_t;

dccthread_t *thread_gerente;
dccthread_t *thread_principal;
dccthread_t *thread_atual;

struct dlist *threads_finalizadas;
struct dlist *threads_prontas;
struct dlist *threads_aguardando;

timer_t timer_preempcao;
struct itimerspec its;
struct sigevent sigenv_timer;
int tempo_preempcao = 10000000;
sigset_t mask;


// Funções Auxiliares
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

void dccthread_yield(){
    sigprocmask(SIG_BLOCK, &mask, NULL);

	dccthread_t *thread_atual = dccthread_self();

	dlist_push_right(threads_prontas, thread_atual);
	swapcontext(&thread_atual->context, &thread_gerente->context);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void dccthread_wait(dccthread_t *tid){
    sigprocmask(SIG_BLOCK, &mask, NULL);

    tid->aguardada = 1;
    if (tid->finalizada == 1) {
        return;
    }
	dccthread_t *thread_em_execucao = dccthread_self();
    thread_em_execucao->thread_aguardada = tid;
    dlist_push_right(threads_aguardando, thread_em_execucao);
    swapcontext(&thread_em_execucao->context, &thread_gerente->context);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
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

static void evento_timer(union sigval sigev_value){
    dccthread_yield();
}

int dccthread_nwaiting(){
    return threads_aguardando->count;
}

int dccthread_nexited(){
    // Percorre a lista de threads finalizadas e conta quantas o campo aguardada é igual a 0
    int count = 0;
    if (threads_finalizadas->count == 0) {
        return count;
    } 
    struct dnode *node = threads_finalizadas->head;
    while (node != NULL) {
        dccthread_t *thread = (dccthread_t *) node->data;
        if (thread->aguardada == 0) {
            count++;
        }
        node = node->next;
    }
    return count;
}

// Funções principais
void dccthread_exit(void){
    sigprocmask(SIG_BLOCK, &mask, NULL);
	dccthread_t* thread_em_execucao = dccthread_self();
    thread_em_execucao->finalizada = 1;
    dlist_push_right(threads_finalizadas, thread_em_execucao);

    if (thread_em_execucao->aguardada == 0) {
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        return;
    }
	dccthread_t* thread_aguardando = (dccthread_t*) dlist_find_remove(threads_aguardando, thread_em_execucao, verifica_thread_aguardada, NULL);
	if(thread_aguardando != NULL){
		dlist_push_right(threads_prontas, thread_aguardando);
	}
   
	setcontext(&thread_gerente->context);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

dccthread_t * dccthread_create(const char *name, void (*func)(int), int param) {
    sigprocmask(SIG_BLOCK, &mask, NULL);

    dccthread_t *nova_thread;
	nova_thread = (dccthread_t*) malloc (sizeof(dccthread_t));
    getcontext(&nova_thread->context);
    strcpy(nova_thread->name, name);
    
    set_initial_context(nova_thread, thread_gerente);

    // Adiciona a thread a lista de threads prontas
    dlist_push_right(threads_prontas, nova_thread);
    makecontext(&nova_thread->context, (void *) func, 1, param);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    return nova_thread;
}

void cria_timer() {
    sigenv_timer.sigev_notify = SIGEV_THREAD;
    sigenv_timer.sigev_signo  = SIGRTMIN;
    sigenv_timer.sigev_notify_function = evento_timer;
    sigenv_timer.sigev_value.sival_ptr = &timer_preempcao;

    timer_create(CLOCK_PROCESS_CPUTIME_ID, &sigenv_timer, &timer_preempcao);
	its.it_value.tv_nsec = tempo_preempcao;
	its.it_interval.tv_nsec = tempo_preempcao;
}

void gerenciador() {
    while(!dlist_empty(threads_prontas)) {
        timer_settime(timer_preempcao, 0, &its, NULL);
        dccthread_t *thread = (dccthread_t *) malloc (sizeof(dccthread_t));
        thread = threads_prontas->head->data;
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        swapcontext(&thread_gerente->context, &thread->context);
        sigprocmask(SIG_BLOCK, &mask, NULL);
        dlist_pop_left(threads_prontas);
    }
    exit(EXIT_SUCCESS);
}

void dccthread_init(void (*func)(int), int param) {
    // Inicializa as listas de threads prontas e threads aguardando
    threads_prontas = dlist_create();
    threads_aguardando = dlist_create();
    threads_finalizadas = dlist_create();

    // Inicializa a mascara de sinais
 	sigemptyset(&mask);
	sigaddset(&mask, SIGRTMIN);
    sigprocmask(SIG_SETMASK, &mask, NULL);	
    sigprocmask(SIG_BLOCK, &mask, NULL);

    // Inicializa o timer
    cria_timer();

    // Cria a thread gerentes
    thread_gerente = (dccthread_t *) malloc (sizeof(dccthread_t));
    getcontext(&thread_gerente->context);
    strcpy(thread_gerente->name, "gerente");
    set_initial_context(thread_gerente, NULL);
    makecontext(&thread_gerente->context, (void *) gerenciador, 0);

    // Cria a thread thread_principal
    thread_principal = dccthread_create("main", func, param);
 
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
 
    setcontext(&thread_gerente->context);
   
    exit(EXIT_SUCCESS);
}