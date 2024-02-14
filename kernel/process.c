#include "process.h"
#include "debug.h"
#include "debugger.h"
#include "string.h"
#include "mem.h"
#include "cpu.h"
#include "queue.h"

static int NEXT_PID = 0;

static Process *PROCESS_TABLE[NBPROC];

static Process *CURRENT_PROCESS;

static link ACTIVABLE_LIST = LIST_HEAD_INIT(ACTIVABLE_LIST);


void ordonnance() {
    if (queue_empty(&ACTIVABLE_LIST)) {
        return;
    }
    Process *old_process = CURRENT_PROCESS;
    old_process->state = ACTIVABLE;
    
    
    Process *new_process = queue_out(&ACTIVABLE_LIST, Process, listfield);

    queue_add(old_process, &ACTIVABLE_LIST, Process, listfield, priority);
    
    new_process->state = ACTIVE;
    CURRENT_PROCESS = new_process;

    ctx_sw((void*)old_process->context, (void*)new_process->context);

}

int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg) {
    Process *process = check_pointer(mem_alloc(sizeof(Process)));
    
    process->pid = NEXT_PID;
    PROCESS_TABLE[process->pid] = process;
    NEXT_PID += 1;

    // TODO erreur si plus de pid dispo
    strncpy(process->name, name, PROCESS_NAME_LEN);

    process->priority = prio;
    if (process->pid == 0) {
        process->state = ACTIVE;
        CURRENT_PROCESS = process;
    } else {
        process->state = ACTIVABLE; // TODO peut être active si prio > au courant?
        queue_add(process, &ACTIVABLE_LIST, Process, listfield, priority);
        // TODO add to activable list
    }

    unsigned long stack_size = ssize + 3;
    process->stack = check_pointer(mem_alloc(stack_size*sizeof(uint32_t)));
    
    process->stack[stack_size-1] = (uint32_t) arg;

    // TODO process->stack[stack_size-2] = exit;

    process->stack[stack_size-3] = (uint32_t) pt_func;
    process->context[1] = (uint32_t) &(process->stack[stack_size-3]);
    return process->pid;
}


