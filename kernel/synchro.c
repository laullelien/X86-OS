#include "synchro.h"
#include "stddef.h"
#include "process.h"
#include "queue.h"

static Pipe *PIPES[NBQUEUE];

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
        Process * process = queue_out(&(file->conso), Process, listfield);
        file->nb_conso--;
        process->return_value = message;
        make_process_activable(process);
    }
    else if (file->taille >= file->taille_max){
        Process * process = getprocess(getpid());

        process->return_value = message;
        queue_add(process, &(file->prod), Process, listfield, no_priority);
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