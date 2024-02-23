#ifndef __TIME_H__
#define __TIME_H__
#include "stdbool.h"
#include <stdint.h>



//extern uint32_t nb_tic;

extern void traitant_IT_32 (void);

void tic_PIT(void);

void init_clock(void);

#endif