#include "debugger.h"
#include "cpu.h"
#include "printf.c"
#include "process.h"
#include "stddef.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
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
	for(;;) {
		printf("proc: %p\n", param);
		sti();
        hlt();
        cli();
	}
}

void kernel_start(void)
{
	int i;
//	call_debugger();

	printf("test\ntoto");
	i = 10;

	i = fact(i);

	printf("test\n%i    toto", i);

	start(idle, 64, 1, "idle", NULL); // TODO maybe 0
	start(proc1, 64, 1, "proc1", (void*)(1235));

	idle(NULL);

	while(1)
	  hlt();

	return;
}
