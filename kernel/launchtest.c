#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "time.h"
#include "screen.h"
#include "start.h"
#include "process.h"
#include "stddef.h"
#include "launchtest.h"

/*  Test 0  */

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

/*  Test 1  */

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

int test1(void *arg){ //prio = 128

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

/*  Test 2  */

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


/*  Test 3  */

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

/*  Test 4  */

void test_it(void)
{
#ifdef microblaze
        int status, mstatus;
        __asm__ volatile("mfs %0,rmsr; ori %1,%0,2; mts rmsr,%1; nop; nop; mts rmsr,%0":"=r" (status), "=r" (mstatus));
#else
        __asm__ volatile("pushfl; testl $0x200,(%%esp); jnz 0f; sti; nop; cli; 0: addl $4,%%esp\n":::"memory");
#endif
}


int busy1(void *arg)
{
        (void)arg;
        while (1) {
                int i, j;

                printf(" A");
                for (i=0; i<loop_count1; i++) {
                        test_it();
                        for (j=0; j<loop_count0; j++);
                }
        }
        return 0;
}
/* assume the process to suspend has a priority == 64 */
int busy2(void *arg)
{
        int i;

        for (i = 0; i < 3; i++) {
                int k, j;

                printf(" B");
                for (k=0; k<loop_count1; k++) {
                        test_it();
                        for (j=0; j<loop_count0; j++);
                }
        }
        i = chprio((int) arg, 16);
        assert(i == 64);
        return 0;
}

int test4(void *args)
{
        int pid1, pid2;
        int r;
        int arg = 0;

        (void)args;

        assert(getprio(getpid()) == 128);
        pid1 = start(busy1, 4000, 64, "busy1",(void *) arg);
        assert(pid1 > 0);
        pid2 = start(busy2, 4000, 64, "busy2", (void *) pid1);
        assert(pid2 > 0);
        printf("1 -");
        r = chprio(getpid(), 32);
        assert(r == 128);
        printf(" - 2");
        r = kill(pid1);
        assert(r == 0);
        assert(waitpid(pid1, 0) == pid1);
        r = kill(pid2);
        assert(r < 0); /* kill d'un processus zombie */
        assert(waitpid(pid2, 0) == pid2);
        printf(" 3");
        r = chprio(getpid(), 128);
        assert(r == 32);
        printf(" 4.\n");

        return 0;
}

/*  Test 5  */

int no_run(void *arg)
{
        (void)arg;
        assert(0);
        return 1;
}

int waiter(void *arg)
{
        int pid = (int)arg;
        assert(kill(pid) == 0);
        assert(waitpid(pid, 0) < 0);
        return 1;
}


int test5(void *arg)
{
        int pid1, pid2;
        int r;

        (void)arg;

        // Le processus 0 et la priorite 0 sont des parametres invalides
        assert(kill(0) < 0);
        assert(chprio(getpid(), 0) < 0);
        assert(getprio(getpid()) == 128);

        pid1 = start(no_run, 4000, 64, "no_run1", 0);
        assert(pid1 > 0);
        assert(kill(pid1) == 0);
        assert(kill(pid1) < 0); //pas de kill de zombie
        assert(chprio(pid1, 128) < 0); //changer la priorite d'un zombie
        assert(chprio(pid1, 64) < 0); //changer la priorite d'un zombie
        assert(waitpid(pid1, 0) == pid1);
        assert(waitpid(pid1, 0) < 0);
        pid1 = start(no_run, 4000, 64, "no_run2", 0);
        assert(pid1 > 0);
        pid2 = start(waiter, 4000, 65, "waiter",(void *)pid1);
        assert(pid2 > 0);
        assert(waitpid(pid2, &r) == pid2);
        assert(r == 1);
        assert(waitpid(pid1, &r) == pid1);
        assert(r == 0);
        printf("ok.\n");
        return 0;   //added
}

// /* Test 6 */

extern int proc6_1(void*);

        #if defined microblaze
        __asm__(
        ".text\n"
        ".globl main\n"
        "main:\n"
        "addik r3,r0,3\n"
        "rtsd r15,8\n"
        "nop\n"
        ".previous\n"
        );
        #else
        __asm__(
        ".text\n"
        ".globl proc6_1\n"
        "proc6_1:\n"
        "movl $3,%eax\n"
        "ret\n"
        ".previous\n"
        );
        #endif


extern int proc6_2(void*);
        #if defined microblaze
        __asm__(
        ".text\n"
        ".globl main\n"
        "main:\n"
        "addk r3,r0,r5\n"
        "swi r3,r1,-4\n"
        "rtsd r15,8\n"
        "nop\n"
        ".previous\n"
        );
        #else
        __asm__(
        ".text\n"
        ".globl proc6_2\n"
        "proc6_2:\n"
        "movl 4(%esp),%eax\n"
        "pushl %eax\n"
        "popl %eax\n"
        "ret\n"
        ".previous\n"
        );
        #endif


extern int proc6_3(void*);
__asm__(
".text\n"
".globl proc6_3\n"
"proc6_3:\n"
"movl 4(%esp),%eax\n"
"pushl %eax\n"
"popl %eax\n"
"ret\n"
".previous\n"
);


int test6(void *arg)
{
        int pid1, pid2, pid3;
        int ret;

        (void)arg;

        assert(getprio(getpid()) == 128);
        pid1 = start(proc6_1, 1000, 64, "proc6_1", 0);
        assert(pid1 > 0);
        pid2 = start(proc6_2, 4000, 66, "proc6_2", (void*)4);
        assert(pid2 > 0);
        pid3 = start(proc6_3, 0xffffffff, 65, "proc6_3",(void*)5);
        assert(pid3 < 0);
        pid3 = start(proc6_3, 8000, 65, "proc6_3",(void*)5);
        assert(pid3 > 0);
        assert(waitpid(-1, &ret) == pid2);
        assert(ret == 4);
        assert(waitpid(-1, &ret) == pid3);
        assert(ret == 5);
        assert(waitpid(-1, &ret) == pid1);
        assert(ret == 3);
        assert(waitpid(pid1, 0) < 0);
        assert(waitpid(-1, 0) < 0);
        assert(waitpid(getpid(), 0) < 0);
        printf("ok.\n");
        return 0; //added
}

/* Test 7 */

// int sleep_pr1(void *arg)
// {
//         (void)arg;
//         wait_clock(current_clock() + 2);
//         printf(" not killed !!!");
//         assert(0);
//         return 1;
// }

// #ifdef TELECOM_TST
// int test7(void *arg)
// {
//         (void)arg;
//         printf("Test desactive pour les TELECOM.\n");
// }

// #else

// int test7(void *arg)
// {
//         int pid1, pid2, r;
//         unsigned long c0, c, quartz, ticks, dur;
//         volatile unsigned long *timer = NULL;

//         (void)arg;
//         timer = shm_create("test7_shm");
//         assert(timer != NULL);

//         assert(getprio(getpid()) == 128);
//         printf("1");
//         pid1 = start(timer1, 4000, 129, "timer1", 0);
//         assert(pid1 > 0);
//         printf(" 3");
//         assert(waitpid(-1, 0) == pid1);
//         printf(" 8 : ");

//         *timer = 0;
//         pid1 = start(timer, 4000, 127, "timer", 0);
//         pid2 = start(timer, 4000, 127, "timer", 0);
//         assert(pid1 > 0);
//         assert(pid2 > 0);
//         clock_settings(&quartz, &ticks);
//         dur = 2 * quartz / ticks;
//         test_it();
//         c0 = current_clock();
//         do {
//                 test_it();
//                 c = current_clock();
//         } while (c == c0);
//         wait_clock(c + dur);
//         assert(kill(pid1) == 0);
//         assert(waitpid(pid1, 0) == pid1);
//         assert(kill(pid2) == 0);
//         assert(waitpid(pid2, 0) == pid2);
//         printf("%lu changements de contexte sur %lu tops d'horloge", *timer, dur);
//         pid1 = start(sleep_pr1, 4000, 192, "sleep_pr1", 0);
//         assert(pid1 > 0);
//         assert(kill(pid1) == 0);
//         assert(waitpid(pid1, &r) == pid1);
//         assert(r == 0);
//         printf(".\n");
//         shm_release("test7_shm");
// }
// #endif

// int timer(void *arg)
// {
//         volatile unsigned long *timer = NULL;
//         timer = shm_acquire("test7_shm");
//         assert(timer != NULL);

//         (void)arg;
//         while (1) {
//                 unsigned long t = *timer + 1;
//                 *timer = t;
//                 while (*timer == t) test_it();
//         }
//         while (1);
//         return 0;
// }

// int timer1(void *arg)
// {
//         (void)arg;

//         unsigned long quartz;
//         unsigned long ticks;
//         unsigned long dur;
//         int i;

//         clock_settings(&quartz, &ticks);
//         dur = (quartz + ticks) / ticks;
//         printf(" 2");
//         for (i = 4; i < 8; i++) {
//                 wait_clock(current_clock() + dur);
//                 printf(" %d", i);
//         }
//         return 0;
// }






////

#define NB_TEST_CASE 7
static int size = NB_TEST_CASE;                 /*Mark null to not execute the test case*/
static int (*test_case[NB_TEST_CASE])(void *) = {test0, test1, test2, test3, NULL, test5, test6};

int launchtest() {
    int pid;
    int ret;
    for (int i = 0; i < size; i++)
    {
        if (test_case[i]!=NULL) 
        {
            printf("Test %i : ", i);
            char name[6] = {'t', 'e', 's', 't', '\0', '\0'};
            name[4] = i + 48;
            pid = start(test_case[i], 4000, 128, name, 0);
            waitpid(pid, &ret);
            assert(ret == 0);
        }
    }
    return 1;
}
