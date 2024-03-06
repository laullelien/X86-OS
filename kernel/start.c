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
        //printf("idle\n");
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

//Test 0

int test0(void *arg)
{
        (void)arg;
        register unsigned reg1 = 1u;
        register unsigned reg2 = 0xFFFFFFFFu;
        register unsigned reg3 = 0xBADB00B5u;
        register unsigned reg4 = 0xDEADBEEFu;


        printf("I'm a simple process running ...");

        unsigned i;
        for (i = 0; i < 10000000; i++) {
             if (reg1 != 1u || reg2 != 0xFFFFFFFFu || reg3 != 0xBADB00B5u || reg4 != 0xDEADBEEFu) {
                printf(" and I feel bad. Bybye ...\n");
                assert(0);
             }
        }

        printf(" and I'm healthy. Leaving.\n");

        return 0;
}

//Test 1

int dummy2(void *arg){
        printf(" 5");
        assert((int) arg == DUMMY_VAL + 1);
        return 4;
}

int dummy1(void *arg) {
        printf("1");
        assert((int) arg == DUMMY_VAL);
        return 3;
}

int test1(void *arg){

	int pid1;
	int r;
	int rval;

	(void)arg;

	pid1 = start(dummy1, 4000, 192, "dummy1",(void *) DUMMY_VAL);
	assert(pid1 > 0);
	printf(" 2");
	r = waitpid(pid1, &rval);
	assert(r == pid1);
	assert(rval == 3);
	printf(" 3");
	pid1 = start(dummy2, 4000, 100, "dummy2", (void *) (DUMMY_VAL + 1));
	assert(pid1 > 0);
	printf(" 4");
	r = waitpid(pid1, &rval);
	assert(r == pid1);
	assert(rval == 4);
	printf(" 6.\n");
	return 0;
}

//Test2

int procExit(void *args)
{
        printf(" 5");
        exit((int) args);
        assert(0);
        return 0;
}

int procKill(void *args)
{
        printf(" X");
        return (int)args;
}

int test2(void *arg)
{
        int rval;
        int r;
        int pid1;
        int val = 45;

        (void)arg;

        printf("1");
        pid1 = start(procKill, 4000, 100, "procKill",(void *) val);
        assert(pid1 > 0);
        printf(" 2");
        r = kill(pid1);
        assert(r == 0);
        printf(" 3");
        r = waitpid(pid1, &rval);
        assert(rval == 0);
        assert(r == pid1);
        printf(" 4");
        pid1 = start(procExit, 4000, 192, "procExit",(void *) val);
        assert(pid1 > 0);
        printf(" 6");
        r = waitpid(pid1, &rval);
        assert(rval == val);
        assert(r == pid1);
        assert(waitpid(getpid(), &rval) < 0);
        printf(" 7.\n");
        return 0; //Added
}


//Test 3

int prio4(void *arg)
{
        /* arg = priority of this proc. */
        int r;

        assert(getprio(getpid()) == (int) arg);
        printf("1");
        r = chprio(getpid(), 64);
        assert(r == (int) arg);
        printf(" 3");
        return 0;
}

int prio5(void *arg)
{
        /* Arg = priority of this proc. */
        int r;

        assert(getprio(getpid()) == (int) arg);
        printf(" 7");
        r = chprio(getpid(), 64);
        assert(r == (int)arg);
        printf("error: I should have been killed\n");
        assert(0);
        return 0;
}


int test3(void *arg)
{
        int pid1;
        int p = 192;
        int r;

        (void)arg;

        assert(getprio(getpid()) == 128);
        pid1 = start(prio4, 4000, p, "prio4",(void *) p);
        assert(pid1 > 0);
        printf(" 2");
        r = chprio(getpid(), 32);
        assert(r == 128);
        printf(" 4");
        r = chprio(getpid(), 128);
        assert(r == 32);
        printf(" 5");
        assert(waitpid(pid1, 0) == pid1);
        printf(" 6");

        assert(getprio(getpid()) == 128);
        pid1 = start(prio5, 4000, p, "prio5",(void *) p);
        assert(pid1 > 0);
        printf(" 8");
        r = kill(pid1);
        assert(r == 0);
        assert(waitpid(pid1, 0) == pid1);
        printf(" 9");
        r = chprio(getpid(), 32);
        assert(r == 128);
        printf(" 10");
        r = chprio(getpid(), 128);
        assert(r == 32);
        printf(" 11.\n");
        return 0; //Added
}

//////



int start_test(void* ) {
        int pid;
        int ret;

	printf("Test %i : ", 0);
	pid = start(test0, 4000,128,"test0",NULL);
        printf("pid = %i",pid);
	waitpid(pid, &ret);
	assert(ret == 0);

	printf("Test %i : ", 1);
	pid = start(test1, 4000,128,"test1",NULL);
	waitpid(pid, &ret);
	assert(ret == 0);


	printf("Test %i : ", 2);
	pid = start(test2, 4000,128,"test2",NULL);
	waitpid(pid, &ret);
	assert(ret == 0);

        printf("Test %i : ", 3);
	pid = start(test3, 4000,128,"test3",NULL);
	waitpid(pid, &ret);
	assert(ret == 0);
        
        return 0;

}

/////////////////////////////////

void kernel_start(void)
{
	init_clock();	
	efface_ecran();

        
	start(idle ,1024,0,"idle",NULL);
	start(start_test ,1024,1,"start_test",NULL);
        
        idle(NULL);
        
	while(1)
	  hlt();

	return;
}





