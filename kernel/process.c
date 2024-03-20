#include "process.h"
#include "debug.h"
#include "debugger.h"
#include "string.h"
#include "mem.h"
#include "cpu.h"
#include "queue.h"
#include "time.h"

extern void exit_asm();

static int NEXT_PID = 0;

static Process *PROCESS_TABLE[NBPROC];

static Process *CURRENT_PROCESS = NULL;

static link ACTIVABLE_LIST = LIST_HEAD_INIT(ACTIVABLE_LIST);

static link KILLED_LIST = LIST_HEAD_INIT(KILLED_LIST);

static link SLEEP_LIST = LIST_HEAD_INIT(SLEEP_LIST);


static void mark_process_killed(Process *process) {

    process->state = KILLED;
    process->priority = 1;
    if (process->brothers_listfield.prev != NULL) {
        queue_del(process, brothers_listfield);
    }

    if (process->listfield.prev != 0 || process->listfield.next != 0) {
        queue_del(process, listfield);
    }

    queue_add(process, &KILLED_LIST, Process, listfield, priority);
    process->queue_head = &KILLED_LIST;

    PROCESS_TABLE[process->pid] = NULL;
}

void make_process_activable(Process *process) {
    process->state = ACTIVABLE;
    queue_add(process, &ACTIVABLE_LIST, Process, listfield, priority);
    process->queue_head = &ACTIVABLE_LIST;
}

Process * getprocess(int pid){
    if (pid >=0 && pid < NBPROC) {
        return PROCESS_TABLE[pid];
    }
    return NULL;
}

static void awake(){
    Process * p;
    while (!queue_empty(&SLEEP_LIST) && (queue_top(&SLEEP_LIST, Process, listfield))->wakeup_time <= current_clock()){
        p = queue_out(&SLEEP_LIST, Process, listfield);
        make_process_activable(p);
    }
}

void ordonnance() {
    awake();
    if (queue_empty(&ACTIVABLE_LIST)) {
        return;
    }
    Process *old_process = CURRENT_PROCESS;
    if (old_process->state == ACTIVE) {
        old_process->state = ACTIVABLE;
        queue_add(old_process, &ACTIVABLE_LIST, Process, listfield, priority);
        old_process->queue_head = &ACTIVABLE_LIST;
    }
    
    Process *new_process = queue_out(&ACTIVABLE_LIST, Process, listfield);    
    new_process->queue_head = NULL;
    if (new_process == old_process) {
        CURRENT_PROCESS->state = ACTIVE;
        return;
    }
    
    new_process->state = ACTIVE;
    CURRENT_PROCESS = new_process;

    ctx_sw((void*)old_process->context, (void*)new_process->context);

}

int start(int (*pt_func)(void*), unsigned long ssize, int prio, const char *name, void *arg) {
    Process *process;

    if(ssize>=MAX_SSIZE){
        return -1;
    }
    
    if (NEXT_PID >= NBPROC) {
        if (queue_empty(&KILLED_LIST)) {
            return -1;
        } else {
            process = queue_out(&KILLED_LIST, Process, listfield);
            process->queue_head = NULL;
            mem_free(process->stack, process->stack_size); // TODO do this in mark_process_killed
            
        }
    } else {
        process = check_pointer(mem_alloc(sizeof(Process)));
        process->pid = NEXT_PID;
        NEXT_PID += 1;
    }

    PROCESS_TABLE[process->pid] = process;
    
    strncpy(process->name, name, PROCESS_NAME_LEN);

    process->priority = prio;

    if (process->pid == 0) {
        process->state = ACTIVE;
        CURRENT_PROCESS = process;
    } else {
        process->state = ACTIVABLE;
        queue_add(process, &ACTIVABLE_LIST, Process, listfield, priority);
        process->queue_head = &ACTIVABLE_LIST;
        
    }

    unsigned long stack_size = ssize + 3;
    process->stack_size = stack_size*sizeof(uint32_t);
    process->stack = check_pointer(mem_alloc(stack_size*sizeof(uint32_t))); // Do not realloc when reusing old process
    
    process->stack[stack_size-1] = (uint32_t) arg;

    process->stack[stack_size-2] = (uint32_t) exit_asm;

    process->stack[stack_size-3] = (uint32_t) pt_func;
    process->context[1] = (uint32_t) &(process->stack[stack_size-3]);

    if (CURRENT_PROCESS == NULL || CURRENT_PROCESS->pid == 0) {
        process->parent = NULL;
    } else {
        process->parent = CURRENT_PROCESS;
        queue_add(process, &(CURRENT_PROCESS->children_list), Process, brothers_listfield, priority);

    }
    
    process->return_value = 0;
    process->children_list.next = &(process->children_list);
    process->children_list.prev = &(process->children_list);

    if (process->pid != 0) {
        ordonnance();
    }

    return process->pid;
}

int getpid(void){
    return CURRENT_PROCESS->pid;
}

static void terminate_process(Process *process) {
    if (process->parent == NULL) {
        mark_process_killed(process);
        
    } else {
        process->state = ZOMBIE;
        if (process->parent->state == WAIT_CHILD) {
            make_process_activable(process->parent);
        }
    }

    Process *child_process;
    Process *previous_process = NULL;

    queue_for_each(child_process, &(process->children_list), Process, brothers_listfield) {
        if (previous_process != NULL) {
            mark_process_killed(previous_process);// TODO ici on enleve le process de la liste de enfants, donc on casse l'itération
            previous_process = NULL;
        }
        child_process->parent = NULL;
        if (child_process->state == ZOMBIE) {
            previous_process = child_process;
            
        }
    }

    if (previous_process != NULL) {
        mark_process_killed(previous_process);// TODO ici on enleve le process de la liste de enfants, donc on casse l'itération
        previous_process = NULL;
    }

    
}


int kill(int pid) {
    if (pid <= 0 || pid >= NBPROC) {
        return -1;
    }
    if (PROCESS_TABLE[pid] == NULL) {
        return -2;
    }
    if (PROCESS_TABLE[pid]->state == ZOMBIE) {
        return -3;
    }
    PROCESS_TABLE[pid]->return_value = 0; 
    terminate_process(PROCESS_TABLE[pid]);
    
    ordonnance();
    return 0;
}

void exit(int retval) {
    assert(CURRENT_PROCESS->pid != 0);

    terminate_process(CURRENT_PROCESS);
    CURRENT_PROCESS->return_value = retval;
    
    ordonnance();
    while (1);
}

int waitpid(int pid, int *retvalp) {
    // TODO waitpid depuis le kernel valide ?
    if (pid >= 0) {
        if (pid < 0 || pid >= NBPROC) {
            return -1;
        }

        if (PROCESS_TABLE[pid] == NULL) {
            return -2;
        }

        if (PROCESS_TABLE[pid]->parent != CURRENT_PROCESS) {
            return -3;
        }

        while (PROCESS_TABLE[pid]->state != ZOMBIE) {
            CURRENT_PROCESS->state = WAIT_CHILD;
            CURRENT_PROCESS->queue_head = NULL;
            ordonnance();
        }

    } else {
        if (queue_empty(&CURRENT_PROCESS->children_list)) {
            return -4;
        }


        while (pid < 0) {
            // TODO make another list for ended processes
            Process *child_process;
            queue_for_each(child_process, &(CURRENT_PROCESS->children_list), Process, brothers_listfield) {
                if (child_process->state == ZOMBIE) {
                    pid = child_process->pid;
                    break;
                }
            }
            if (pid >= 0) {
                break;
            }
            CURRENT_PROCESS->state = WAIT_CHILD;
            CURRENT_PROCESS->queue_head = NULL;
            ordonnance();
        }
    }

    if (retvalp != NULL) {
        *retvalp = PROCESS_TABLE[pid]->return_value;
    }
    mark_process_killed(PROCESS_TABLE[pid]);
    return pid;
}
int getprio(int pid){
    Process * process = getprocess(pid);
    if (process == NULL)
        return -1;

    return process->priority;
}

int chprio(int pid, int newprio){
    Process *process = getprocess(pid);
    if (process == NULL) {
        return -1;
    }
    if (newprio==0){
        return -2;
    }
    if (process->state==ZOMBIE){
        return -3;
    }
    if (process->state==KILLED){
        return -4;
    }


    int oldprio = process->priority;
    if (newprio != oldprio){
        process->priority = newprio;
        if (process->queue_head != NULL){
            queue_del(process, listfield);
            queue_add(process, process->queue_head, Process, listfield, priority);
        }
    }
    ordonnance();
    return oldprio;
}

void wait_clock(unsigned long clock){
    Process * cur = CURRENT_PROCESS;

    cur->state = SLEEP;

    cur->queue_head = &SLEEP_LIST;

    cur->wakeup_time = current_clock() + clock;

    queue_add(cur, &SLEEP_LIST, Process, listfield, wakeup_time);

    ordonnance();
}
