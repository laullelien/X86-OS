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


int idle(void *) {
    for(;;) {
		printf("idle\n");
        sti();
        hlt();
        cli();
    }
}

int proc2(void *param){
	param=param;
	for (int i=0;i<5;i++){
		printf("proc2\n");
		sti();
        hlt();
        cli();
	}

	return 987;
}

int proc1(void *param) {
	int i=1;
	for(;;) {
		printf("%i proc: %p    %i\n", i, param, getpid());
		i ++;
		if (i % 5 == 0) {
			
			int ptr=0;
			int pid = start(proc2, 1024, 0, "proc2", (void*)(12435));
			printf("ret %i, pid %i\n", ptr, pid);
			pid = kill(pid);
			waitpid(-1, NULL);
			
			//pid = waitpid(pid, &ptr);
			printf("ret %i, pid %i\n", ptr, pid);
			i = 1;
		}
		
		
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

	start(idle, 1024, 0, "idle", NULL);
	start(proc1, 1024, 1, "proc1", (void*)(1235));

	idle(NULL);

	while(1)
	  hlt();

	return;
}
