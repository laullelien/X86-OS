#ifndef __SYNCHRO_H__
#define __SYNCHRO_H__

#include "queue.h"

#define NBQUEUE 8

typedef struct _Pipe{
    int fid;
    
    link conso;
    int nb_conso;
    link prod;
    int nb_prod;

    int *messages;
    int deb, taille;
    int taille_max;

    link listfield;

} Pipe;

int pcreate(int count);
int pdelete(int fid);
int preset(int fid);

/*

    pcreate : crée une file de messages
    pdelete : détruit une file de messages
    psend : dépose un message dans une file
    preceive : retire un message d'une file
    preset : réinitialise une file
    pcount : renvoie l'état courant d'une file

*/

#endif
