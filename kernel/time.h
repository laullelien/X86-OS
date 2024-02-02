#ifndef __TIME_H__
#define __TIME_H__
#include "stdbool.h"
#include <stdint.h>


extern uint32_t nb_tic;

extern void print_time(char *str,uint32_t size);

extern void tic_PIT(void);

extern void init_traitant_IT(uint32_t num_IT, void (*traitant)(void));

extern void set_freq(int freq);

extern void mask_IRQ(uint32_t num_IRQ, bool maskPar);

#endif