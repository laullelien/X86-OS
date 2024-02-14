#include "debugger.h"
#include "cpu.h"
#include "printf.c"
#include "stdbool.h"
#include "time.h"
#include "screen.h"
#include "start.h"
#include "process.h"
#include "stddef.h"

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


int idle(void *) {
    for(;;) {
		printf("idle\n");
        sti();
        hlt();
        cli();
    }
}

int proc1(void *param) {
	int i=1;
	for(;;) {
		printf("%i proc: %p\n", i, param);
		i ++;
		
		sti();
        hlt();
        cli();
	}
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

	printf("test\n%i    toto", i);

	start(idle, 128, 1, "idle", NULL); // TODO maybe 0
	start(proc1, 1024, 1, "proc1", (void*)(1235));

	idle(NULL);

	while(1)
	  hlt();

	return;
}
