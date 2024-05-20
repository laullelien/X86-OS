#ifndef __PROCESS_H_INCLUDED__
#define __PROCESS_H_INCLUDED__

#include "stdint.h"
#include "queue.h"
#include "user_alloc.h"

#define NBPROC 24 
  // Nombre de processus max

#define PROCESS_NAME_LEN 20

#define MAX_SSIZE (1024*0x1000)
// correspond à une page table complête

#define KERNEL_STACK_SIZE 4096

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
    uint32_t kernel_stack[2*KERNEL_STACK_SIZE];
    unsigned long stack_size;
    int priority;

    unsigned long wakeup_time;  

    struct _Process *parent;

    link children_list;
    link brothers_listfield;

    int return_value;
    int pipe_success;
    link listfield;

    link *queue_head;
    PageDirectory page_directory;

} Process;

extern void cr3_sw(void *); // page directory
extern void ctx_sw(void *, void *); // old, new

Process * getprocess(int pid);

void ordonnance();

//int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg);
int start(const char *name, unsigned long ssize, int prio, void *arg);

int getpid(void);

int getprio(int pid);

int chprio(int pid, int newprio);

int kill(int pid);
void exit(int retval);
int waitpid(int pid, int *retvalp);
void wait_clock(unsigned long clock);

void make_process_activable(Process *process);

int idle(void *);
int create_idle();

void handle_user_pagefault();

int check_user_pointer(const void *pointer);
void sys_info(char *output);

#endif /* __PROCESS_H_INCLUDED__ */