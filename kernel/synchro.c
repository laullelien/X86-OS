#include "synchro.h"
#include "stddef.h"
#include "process.h"
#include "queue.h"

static Pipe *PIPES[NBQUEUE];

int preceive(int fid, int *message)
{
    if (PIPES[fid] == NULL) {
        return -1;
    }
    Pipe *pipe = PIPES[fid];
    if (pipe->taille == 0) {
        Process * process = getprocess(getpid());
        queue_add(process, &(pipe->conso), Process, listfield, priority);
        process->state = WAIT_MESSAGE;
        pipe->nb_conso++;
        ordonnance();
        return process->return_value;
    }
    else {
        if (message != NULL) {
            *message = pipe->messages[pipe->deb];
        }
        pipe->deb = (pipe->deb + 1) % pipe->taille_max;
        --pipe->taille;
    }

    // La pile était pleine
    if (pipe->taille == pipe->taille_max - 1) {
        if (pipe->nb_prod > 0) {
            Process *prod = queue_top(&pipe->prod, Process, listfield);
            make_process_activable(prod);
            ordonnance();
            pipe->messages[(pipe->deb + pipe->taille) % pipe->taille_max] = prod->return_value;
            pipe->taille++;
            prod->return_value = 0;
            pipe->nb_prod--;
        }
    }
    return 0;
}