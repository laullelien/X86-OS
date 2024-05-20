#include "debugger.h"
#include "cpu.h"
#include "printf.c"
#include "stdbool.h"
#include "time.h"
#include "screen.h"
#include "start.h"
#include "process.h"
#include "stddef.h"
#include "synchro.h"


void kernel_start(void)
{
	init_clock();	
	efface_ecran();
	init_shm();

	create_idle();

	//start("autotest", 1024, 128, NULL);
	start("test_app", 1024, 128, NULL);
	
	idle(NULL);
        
	while(1)
	  hlt();

	return;
}





