#ifndef __SYNCHRO_H__
#define __SYNCHRO_H__

#include "queue.h"

#define NBQUEUE 64

typedef struct _Pipe
{
    int fid;

    link conso;
    link prod;

    int *messages;
    int deb;
    int taille;
    int taille_max;

    link listfield;

} Pipe;

int pcreate(int count);
int pdelete(int fid);
int preset(int fid);

int preceive(int fid, int *message);
int psend(int fid, int message);
int pcount(int fid, int *count);

void *shm_create(const char *key);
void *shm_acquire(const char *key);
void shm_release(const char *key);
void init_shm();

#endif
