#include "synchro.h"
#include "stddef.h"
#include "process.h"
#include "queue.h"
#include "debugger.h"
#include "mem.h"
#include "hash.h"
#include "string.h"


static Pipe *PIPES[NBQUEUE];
static int NEXT_FID = 0;
static link PIPES_QUEUE = LIST_HEAD_INIT(PIPES_QUEUE);

static hash_t SHM_TABLE;

int pcreate(int count) {
    if (count <= 0 || count >= 1<<29) {
        return -1;
    }
    int *messages =  mem_alloc((size_t)count * sizeof(int));
    if (messages == NULL) {
        return -2;
    }

    Pipe *pipe;

    if (NEXT_FID >= NBQUEUE) {
        if (queue_empty(&PIPES_QUEUE)) {
            return -3;
        }
        pipe = queue_out(&PIPES_QUEUE, Pipe, listfield);
    } else {
        pipe = mem_alloc(sizeof(Pipe));
        if (pipe == NULL) {
            mem_free(messages, count * sizeof(int));
            return -4;
        }
        pipe->fid = NEXT_FID;
        NEXT_FID += 1;
        
    }

    pipe->conso.next = &(pipe->conso);
    pipe->conso.prev = &(pipe->conso);

    pipe->prod.next = &(pipe->prod);
    pipe->prod.prev = &(pipe->prod);

    PIPES[pipe->fid] = pipe;
    pipe->messages = messages;

    pipe->deb = 0;
    pipe->taille = 0;
    pipe->taille_max = count;
    return pipe->fid;
}

static void empty_pipe(Pipe *pipe) {
    
    while (!queue_empty(&(pipe->conso))) {
        Process *conso = queue_out(&(pipe->conso), Process, listfield);
        make_process_activable(conso);
        conso->pipe_success = -1;
    }
    queue_empty(&(pipe->conso));

    while (!queue_empty(&(pipe->prod))) {
        Process *prod = queue_out(&(pipe->prod), Process, listfield);
        make_process_activable(prod);
        prod->pipe_success = -1;
    }
    queue_empty(&(pipe->prod));
    
    pipe->deb = 0;
    pipe->taille = 0;
}

int pdelete(int fid) {
    if (fid < 0 || fid >= NBQUEUE) {
        return -1;
    }
    if (PIPES[fid] == NULL) {
        return -2;
    }
    
    Pipe *pipe = PIPES[fid];
    PIPES[fid] = NULL;
    
    mem_free(pipe->messages, pipe->taille_max * sizeof(int));
    
    empty_pipe(pipe);

    queue_add(pipe, &PIPES_QUEUE, Pipe, listfield, deb);
    
    ordonnance(); // donner la main à un processus débloqué qui aurait une plus haute priorité
    
    return 0;
}


int preset(int fid) {
    if (fid < 0 || fid >= NBQUEUE) {
        return -1;
    }
    if (PIPES[fid] == NULL) {
        return -2;
    }
    
    empty_pipe(PIPES[fid]);

    ordonnance(); // donner la main à un processus débloqué qui aurait une plus haute priorité

    return 0;
}

Pipe *get_file(int fid){
    if (fid >= 0 && fid < NBQUEUE)
        return PIPES[fid];
    return NULL;
}

int pcount(int fid, int *count){
    if (check_user_pointer(count)) {return-1;}
    Pipe * file = get_file(fid);
    if (file == NULL)
        return -1;

    if (count!=NULL){
        *count = file->taille;
        Process *process;
        queue_for_each(process, &(file->conso), Process, listfield) {
            *count -= 1;
        }

        queue_for_each(process, &(file->prod), Process, listfield) {
            *count += 1;
        }
    }
    return 0;
}

int psend(int fid, int message){
    Pipe * file = get_file(fid);
    if (file == NULL) {
        return -1;
    }
    
    if(file->taille == 0 && !queue_empty(&(file->conso))){
        Process *process = queue_out(&(file->conso), Process, listfield);
        process->queue_head = NULL;
        process->return_value = message; // le consommateur prendra message de return_value et l'inserera dans le tableau
        make_process_activable(process);
        ordonnance();
        return 0;
    } else if (file->taille >= file->taille_max){ //pile pleine
        Process * process = getprocess(getpid());

        process->return_value = message;
        queue_add(process, &(file->prod), Process, listfield, priority);
        process->queue_head = &(file->prod);
        
        process->state = WAIT_MESSAGE;
        process->pipe_success = 0;
        ordonnance();
        return process->pipe_success;
    } else { 
        file->messages[(file->deb + file->taille)%file->taille_max] = message;
        file->taille++;
    }
    return 0;
}


int preceive(int fid, int *message) {
    if (check_user_pointer(message)) {return -1;}
    Pipe * pipe = get_file(fid);
    if (pipe == NULL) {
        return -1;
    }

    if (pipe->taille == 0) {
        Process *process = getprocess(getpid());
        queue_add(process, &(pipe->conso), Process, listfield, priority);
        process->queue_head = &(pipe->conso);

        process->state = WAIT_MESSAGE;
        process->pipe_success = 0;
        ordonnance();
        if (message != NULL) {
            *message = process->return_value; 
        }
        return process->pipe_success;
    } else {
        if (message != NULL) {
            *message = pipe->messages[pipe->deb];
        }
        pipe->deb = (pipe->deb + 1) % pipe->taille_max;
        pipe->taille -= 1;
    }

    // Si la pile était pleine il ffaut reveiller un producteur en attente
    if (pipe->taille >= pipe->taille_max - 1) {
        if (!queue_empty(&(pipe->prod))) {
            Process *prod = queue_out(&pipe->prod, Process, listfield);
            make_process_activable(prod);
            pipe->messages[(pipe->deb + pipe->taille) % pipe->taille_max] = prod->return_value;
            pipe->taille++;
            ordonnance();
        }
    }
    return 0;
}



// SHM

typedef struct _ShmObject {
    int count;
    void *address;
    char *key;
    void *user_address;
    int page_index;

} ShmObject;

static int current_shm_index = 0;



void *shm_create(const char *key) {
    if (check_user_pointer(key)) {return NULL;}
    if (key == NULL) {
        return NULL;
    }

    if (hash_isset(&SHM_TABLE, (void *)key)) {
        return NULL;
    }

    ShmObject *shm = mem_alloc(sizeof(ShmObject));
    if (shm == NULL) {
        return NULL;
    }
    shm->address = user_alloc();
    if (shm->address == NULL) {
        mem_free(shm, sizeof(ShmObject));
        return NULL;
    }
    shm->key = mem_alloc(strlen(key)+1);
    if (shm->key == NULL) {
        mem_free(shm, sizeof(ShmObject));
        user_free(shm->address);
    }

    shm->count = 0;
    shm->page_index = current_shm_index;
    current_shm_index ++;

    shm->user_address = (void *)((1<<31) + shm->page_index*(1<<12));
    
    strcpy(shm->key, key);

    hash_set(&SHM_TABLE, (void *)shm->key, shm);

    return shm_acquire(key);
}
void *shm_acquire(const char *key) {
    if (check_user_pointer(key)) {return NULL;}
    ShmObject *shm = hash_get(&SHM_TABLE, (void *)key, NULL);
    if (shm == NULL) {
        return NULL;
    }
    Process *process = getprocess(getpid());
    if (process->page_directory.shm_page_table[shm->page_index] == 0) {
        // ce process n'a pas encore ouvert ce shm
        shm->count ++;
    }

    process->page_directory.shm_page_table[shm->page_index] = ((uint32_t)shm->address)|0x7;
    cr3_sw((void*) process->page_directory.address); // il faut forcer la mase à jour du page directory 
    return shm->user_address;
}

void shm_release(const char *key) {
    if (check_user_pointer(key)) {return;}
    ShmObject *shm = hash_get(&SHM_TABLE, (void *)key, NULL);
    
    if (shm == NULL) {
        return;
    }
    Process *process = getprocess(getpid());
    if (process->page_directory.shm_page_table[shm->page_index] == 0) {
        // ce process n'a pas ouvert ce shm
        return;
    }
    shm->count --;
    
    process->page_directory.shm_page_table[shm->page_index] = 0;
    cr3_sw((void*) process->page_directory.address); // il faut forcer la mase à jour du page directory 

    if (shm->count <= 0) {
        hash_del(&SHM_TABLE, (void *)key);
        mem_free(shm->key, strlen(shm->key)+1);
        user_free(shm->address);
        mem_free(shm, sizeof(ShmObject));
    }
}

void init_shm() {
    hash_init_string(&SHM_TABLE);
}
