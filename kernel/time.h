#ifndef __TIME_H__
#define __TIME_H__
#include "stdbool.h"
#include <stdint.h>


extern void traitant_IT_32 (void);

void tic_PIT(void);

void init_clock(void);

unsigned long current_clock();
void clock_settings(unsigned long *quartz, unsigned long *ticks);

#endif