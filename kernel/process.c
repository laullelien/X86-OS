#include "process.h"
#include "debug.h"
#include "string.h"

int NEXT_PID = 1;


int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg) {
    Process *process = check_pointer(calloc(1, sizeof(Process)));
    
    process->pid = NEXT_PID;
    NEXT_PID += 1;
    // TODO erreur si plus de pid dispo
    strncpy(process->name, name, PROCESS_NAME_LEN);

    process->priority = prio;
    process->state = ACVTIVABLE; // TODO peut être active si prio > au courant?

    unsigned long stack_size = ssize + 3;
    process->stack = check_pointer(malloc(stack_size*sizeof(uint32_t)));
    
    process->stack[stack_size-1] = (uint32_t) arg;

    // TODO process->stack[stack_size-2] = exit;

    process->stack[stack_size-3] = pt_func;
    process->context[1] = (uint32_t) &(process->stack[stack_size-3])
    return process->pid;
}


