#ifndef __PROCESS_H_INCLUDED__
#define __PROCESS_H_INCLUDED__

#include "stdint.h"
#include "queue.h"

#define NBPROC 30   // Nombre de processus max

#define PROCESS_NAME_LEN 20

enum PROCESS_STATE {
    ACTIVE,
    ACTIVABLE,
    WAIT_MESSAGE,
    WAIT_IO,
    WAIT_CHILD,
    SLEEP,
    ZOMBIE,
    KILLED
};

typedef struct _Process{
    int pid;
    char name[PROCESS_NAME_LEN+1];
    enum PROCESS_STATE state;
    uint32_t context[5];
    uint32_t *stack;
    unsigned long stack_size;
    int priority;

    int no_priority;
    
    struct _Process *parent;

    link children_list;
    link brothers_listfield;

    int return_value;
    link listfield;

} Process;

extern void ctx_sw(void *, void *);

Process * getprocess(int pid);

void ordonnance();

int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg);

int getpid(void);

int getprio(int pid);

int chprio(int pid, int newprio);

int kill(int pid);
void exit(int retval);
int waitpid(int pid, int *retvalp);

void make_process_activable(Process *process);

#endif /* __PROCESS_H_INCLUDED__ */