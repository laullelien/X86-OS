#include "synchro.h"
#include "stddef.h"
#include "process.h"
#include "queue.h"
#include "queue.h"
#include "debugger.h"
#include "stddef.h"
#include "process.h"
#include "mem.h"


static Pipe *PIPES[NBQUEUE];
static int NEXT_FID = 0;
static link PIPES_QUEUE = LIST_HEAD_INIT(PIPES_QUEUE);


int pcreate(int count) {
    if (count <= 0) {
        return -1;
    }

    Pipe *pipe;

    if (NEXT_FID >= NBQUEUE) {
        if (queue_empty(&PIPES_QUEUE)) {
            return -2;
        }
        pipe = queue_out(&PIPES_QUEUE, Pipe, listfield);
    } else {
        pipe = check_pointer(mem_alloc(sizeof(Pipe)));
        pipe->fid = NEXT_FID;
        NEXT_FID += 1;
        pipe->conso.next = &(pipe->conso);
        pipe->conso.prev = &(pipe->conso);

        pipe->prod.next = &(pipe->prod);
        pipe->prod.prev = &(pipe->prod);

    }

    PIPES[pipe->fid] = pipe;
    pipe->messages = check_pointer(mem_alloc(count * sizeof(int)));
    pipe->nb_prod = 0;
    pipe->nb_conso = 0;
    pipe->deb = 0;
    pipe->taille = 0;
    pipe->taille_max = count;
    return pipe->fid;
}

static void empty_pipe(Pipe *pipe) {
    Process *conso;
    queue_for_each(conso, &(pipe->conso), Process, listfield) {
        make_process_activable(conso);
        conso->return_value = -1;
    }
    queue_empty(&(pipe->conso));
    pipe->nb_conso = 0;

    Process *prod;
    queue_for_each(prod, &(pipe->prod), Process, listfield) {
        make_process_activable(prod);   
        prod->return_value = -1;
    }
    queue_empty(&(pipe->prod));
    pipe->nb_prod = 0;
    
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
    
    Pipe *pipe = PIPES[fid];
    
    empty_pipe(pipe);
    
    ordonnance(); // donner la main à un processus débloqué qui aurait une plus haute priorité

    return 0;
}

Pipe * get_file(int fid){
    if (fid >= 0 && fid < NBQUEUE)
        return PIPES[fid];
    return NULL;
}

int psend(int fid, int message){
    Pipe * file = get_file(fid);
    if (file == NULL)
        return -1;
    
    if(file->taille == 0 && !queue_empty(&(file->conso))){
        Process *process = queue_out(&(file->conso), Process, listfield);
        process->queue_head = NULL;
        file->nb_conso--;
        process->return_value = message;
        make_process_activable(process);
        ordonnance();
        return 0;
    }
    else if (file->taille >= file->taille_max){
        Process * process = getprocess(getpid());

        process->return_value = message;
        queue_add(process, &(file->prod), Process, listfield, priority);
        process->queue_head = &(file->prod);
        
        process->state = WAIT_MESSAGE;
        file->nb_prod++;
        ordonnance();
        return process->return_value;
    }
    else { /*(file->taille < file->taille_max)*/
        file->messages[(file->deb + file->taille)%file->taille_max] = message;
        file->taille++;
    }
    return 0;
}

int pcount(int fid, int *count){
    Pipe * file = get_file(fid);
    if (file == NULL)
        return -1;

    if (count!=NULL){
        *count = file->taille + file->nb_prod - file->nb_conso;
    }
    return 0;
}