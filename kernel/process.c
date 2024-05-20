#include "process.h"
#include "debug.h"
#include "debugger.h"
#include "string.h"
#include "mem.h"
#include "cpu.h"
#include "queue.h"
#include "time.h"
#include "userspace_apps.h"
#include "user_alloc.h"
#include "segment.h"

extern void exit_asm();
extern void start_user_process(uint32_t EIP, uint32_t CS, uint32_t EFLAGS, uint32_t ESP, uint32_t SS);

static int NEXT_PID = 0;

static Process *PROCESS_TABLE[NBPROC]; // L'indice est le pid. Contient NULL si le process est KILLED ou pas encore créé.

static Process *CURRENT_PROCESS = NULL;

static link ACTIVABLE_LIST = LIST_HEAD_INIT(ACTIVABLE_LIST);

static link KILLED_LIST = LIST_HEAD_INIT(KILLED_LIST);

static link SLEEP_LIST = LIST_HEAD_INIT(SLEEP_LIST);

int check_user_pointer(const void *pointer) {
	if (pointer != NULL && (uint32_t)pointer < 1u<<30) {
        // Il pourrait être interessant de tuer le process s'il essaye de tricher, mais on ne le fait pas pour les tests
		// printf("\n##Pointer in parameter is not in user range, killing process %i\n", getpid());
		// kill(getpid());
        return 1;
	}
    return 0;
}

static const struct uapps *get_uapp(const char *name) {
    int i = 0;
    while (symbols_table[i].name != NULL) {
        if (strcmp(symbols_table[i].name, name) == 0) {
            return &symbols_table[i];
        }
        i ++;
    }
    return NULL;
}

static void mark_process_killed(Process *process) {

    process->state = KILLED;
    process->priority = 1;
    if (process->brothers_listfield.prev != 0 || process->brothers_listfield.next != 0) {
        queue_del(process, brothers_listfield);
    }
    
    if (process->listfield.prev != 0 || process->listfield.next != 0) {
        queue_del(process, listfield);
    }

    queue_add(process, &KILLED_LIST, Process, listfield, priority);
    process->queue_head = NULL;
    PROCESS_TABLE[process->pid] = NULL;
}

void make_process_activable(Process *process) {
    process->state = ACTIVABLE;
    queue_add(process, &ACTIVABLE_LIST, Process, listfield, priority);
    process->queue_head = &ACTIVABLE_LIST;
}

Process *getprocess(int pid){
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
    
    // On change le page directory vers celui du nouveau process
    cr3_sw((void*) new_process->page_directory.address);
    
    // Placer le sommet de la pile kernel du nouveau process dans la TSS
    *(uint32_t*)(0x20004) = (uint32_t) &(new_process->kernel_stack[KERNEL_STACK_SIZE-3]);//((uint32_t) new_process->context[1]);//+4*42;
    *(uint32_t*)(0x20008) = KERNEL_DS;

    ctx_sw((void*) old_process->context, (void*)new_process->context);
}


int idle(void *) {
    for(;;) {
        //printf("idle\n");
        
        sti();
        hlt();
        cli();
    }
}

int create_idle(){
    // La création d'idle est plus simple que les autres process puisqu'on a pas besoin du côté user
    Process *process;
    assert(NEXT_PID == 0); // idle doit être appelé en premier

    process = check_pointer(mem_alloc(sizeof(Process)));
    process->pid = NEXT_PID;
    NEXT_PID += 1;

    PROCESS_TABLE[process->pid] = process;
    
    strncpy(process->name, "idle", PROCESS_NAME_LEN);

    process->priority = 0;
    
    process->state = ACTIVE;
    CURRENT_PROCESS = process;
    
    process->stack = NULL;
    process->stack_size = 0;

    process->page_directory.address = pgdir;

    process->context[1] = (uint32_t) &(process->kernel_stack[KERNEL_STACK_SIZE-3]);
    
    process->kernel_stack[KERNEL_STACK_SIZE-1] = 0;

    process->kernel_stack[KERNEL_STACK_SIZE-2] = 0; // On ne return jamais de idle 

    process->kernel_stack[KERNEL_STACK_SIZE-3] = (uint32_t) idle;
    
    process->parent = NULL;
    
    process->return_value = 0;
    process->children_list.next = &(process->children_list);
    process->children_list.prev = &(process->children_list);

    return process->pid;
}

int process_start(void* arg) {
    // On place l'argument au bon endroit de la pile 
    *(CURRENT_PROCESS->stack-10) = (uint32_t) arg;
    
    start_user_process(1u<<30, USER_CS, 0x202, (1u<<31)-(12*sizeof(uint32_t)), USER_DS);
    
    return 0; // on ne passe jamais ici
}

int start(const char *name, unsigned long ssize, int prio, void *arg){
    if (NEXT_PID == 0) {
        return -1; // idle doit être appelé en premier
    }

    const struct uapps *uapp = get_uapp(name);
    if (uapp == NULL) {
        return -2;
    }

    Process *process;

    if(ssize >= MAX_SSIZE){
        return -3;
    }

    if (strlen(name) >= PROCESS_NAME_LEN) {
        return -4;
    }
    
    if (NEXT_PID >= NBPROC) {
        // On doit réutiliser un ancien process
        if (queue_empty(&KILLED_LIST)) {
            return -5; // Aucun process n'est disponible
        } else {
            process = queue_out(&KILLED_LIST, Process, listfield);
            process->queue_head = NULL;           
        }
    } else {
        process = check_pointer(mem_alloc(sizeof(Process)));
        process->pid = NEXT_PID;
        NEXT_PID += 1;
        process->stack = init_page_directory(&process->page_directory);
    }
    
    map_user_code(&process->page_directory, uapp);
    
    PROCESS_TABLE[process->pid] = process;
    
    strncpy(process->name, name, PROCESS_NAME_LEN);

    process->priority = prio;
    process->state = ACTIVABLE;
    //printf("\n%p, %p, %p, %p, %x, %p\n", process, &process->listfield, process->listfield.prev, process->listfield.next, CURRENT_PROCESS->context[1],CURRENT_PROCESS->kernel_stack);
    queue_add(process, &ACTIVABLE_LIST, Process, listfield, priority); // UN BUG ICI ?
    process->queue_head = &ACTIVABLE_LIST;
    
    // on stocke l'adresse du haut de la pile dans le bon registre du contexte
    process->context[0] = 0;
    process->context[1] = (uint32_t) &(process->kernel_stack[KERNEL_STACK_SIZE-3]);
    process->context[2] = 0;
    process->context[3] = 0;
    process->context[4] = 0;
        
    process->kernel_stack[KERNEL_STACK_SIZE-1] = (uint32_t) arg;

    process->kernel_stack[KERNEL_STACK_SIZE-2] = (uint32_t) exit_asm; // inutile en pratique car on ne va jamais retourner de la fonction

    process->kernel_stack[KERNEL_STACK_SIZE-3] = (uint32_t) process_start; // la fonction à appeler
    

    if (CURRENT_PROCESS == NULL || CURRENT_PROCESS->pid == 0) {
        process->parent = NULL;
    } else {
        process->parent = CURRENT_PROCESS;
        queue_add(process, &(CURRENT_PROCESS->children_list), Process, brothers_listfield, priority);
    }
    
    process->return_value = 0;
    process->children_list.next = &(process->children_list);
    process->children_list.prev = &(process->children_list);

    if (CURRENT_PROCESS->pid != 0) {
        ordonnance();
    }

    return process->pid;
}

int getpid(void){
    return CURRENT_PROCESS->pid;
}

static void terminate_process(Process *process) {
    if (process->listfield.prev != 0 || process->listfield.next != 0) {
        queue_del(process, listfield);
    }
    
    if (process->parent == NULL) { // le parent ne peut pas être ZOMBIE car sinon le parent est placé à NULL
        mark_process_killed(process);
    } else {
        process->state = ZOMBIE;
        if (process->parent->state == WAIT_CHILD) {
            make_process_activable(process->parent);
        }
    }

    while (!queue_empty(&(process->children_list))) {
        Process *child_process = queue_out(&(process->children_list), Process, brothers_listfield);
        child_process->parent = NULL;
        if (child_process->state == ZOMBIE) {
            mark_process_killed(child_process);
        }
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
    if (check_user_pointer(retvalp)){return -1;}
    if (pid >= 0) {
        if (pid <= 0 || pid >= NBPROC) {
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
    if (process == NULL) {
        return -1;
    }

    return process->priority;
}

int chprio(int pid, int newprio){
    Process *process = getprocess(pid);
    if (process == NULL) {
        return -1;
    }
    if (newprio <= 0){
        return -2;
    }
    if (process->state == ZOMBIE){
        return -3;
    }
    if (process->state == KILLED){
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

    cur->queue_head = NULL;

    cur->wakeup_time = clock;

    queue_add(cur, &SLEEP_LIST, Process, listfield, wakeup_time);

    ordonnance();
}


void handle_user_pagefault() {
    outb(0x20, 0x20);
    printf("\n## Pagefault detected, killing process %i\n", getpid());
    kill(getpid());
}

void sys_info(char *output) {
    if (check_user_pointer(output)){return;}
    output[0] = 0;
    for (int i=0;i<NBPROC;i++) {
        char str[50];
        char *state;
        char *name;
        if (PROCESS_TABLE[i] == NULL) {
            name = "";
            state = "KILLED";
        } else {
            name = PROCESS_TABLE[i]->name;
            switch (PROCESS_TABLE[i]->state) {
                case KILLED:
                    state = "KILLED";
                    break;
                case ACTIVE:
                    state = "ACTIVE";
                    break;
                case ACTIVABLE:
                    state = "ACTIVABLE";
                    break;
                case WAIT_MESSAGE:
                    state = "WAIT_MESSAGE";
                    break;
                case WAIT_IO:
                    state = "WAIT_IO";
                    break;
                case WAIT_CHILD:
                    state = "WAIT_CHILD";
                    break;
                case SLEEP:
                    state = "SLEEP";
                    break;
                case ZOMBIE:
                    state = "ZOMBIE";
                    break;
                default:
                    break;
            }
        }
        sprintf(str, "\n- pid \t%i \t- \t%s \t- \t%s", i, name, state);
        strcat(output, str);
    }
}