#include "debugger.h"
#include "cpu.h"
#include "printf.c"
#include <stdbool.h>
#include "time.h"
#include "screen.h"
#include "start.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}

void init_clock(void){
	init_traitant_IT(32,traitant_IT_32);
    set_freq(50);
    mask_IRQ(0,0);
}


void kernel_start(void)
{
	init_clock();	
	efface_ecran();
	
	int i;
//	call_debugger();
	
	printf("test\ntoto");
	i = 10;

	i = fact(i);
	sti();
	while(1)
	  hlt();

	return;
}
