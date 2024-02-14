#ifndef __PROCESS_H_INCLUDED__
#define __PROCESS_H_INCLUDED__

#include "stdint.h"
#include "queue.h"

#define NBPROC 30   // Nombre de processus max

#define PROCESS_NAME_LEN 20

enum PROCESS_STATE {
    ACTIVE,
    ACTIVABLE,
    PRECEIVE,
    WAIT, 
    WAIT_IO,
    WAIT_CHILD,
    SLEEP,
    ZOMBIE
};

typedef struct _Process{
    int pid;
    char name[PROCESS_NAME_LEN+1];
    enum PROCESS_STATE state;
    uint32_t context[5];
    uint32_t *stack;
    int priority;

    link listfield;

} Process;

extern void ctx_sw(void *, void *);

void ordonnance();

int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg);


/*
 TODO
    start : crée un processus dans l'état activable.
    getpid : récupére l'identifiant du processus actif.
    getprio : lit la priorité d'un processus.
    chprio : modifie la priorité d'un processus.

    exit : termine le processus actif (ie. soi-même).
    kill : met fin à un processus.
    waitpid : attend la terminaison d'un processus fils et récupère sa valeur de retour.
    
    
    

*/


#endif /* __PROCESS_H_INCLUDED__ */