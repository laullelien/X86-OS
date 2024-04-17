#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "time.h"
#include "screen.h"
#include "start.h"
#include "process.h"
#include "stddef.h"
#include "launchtest.h"
#include "synchro.h"
#include "div64.h"
#include "string.h"

typedef unsigned long long uint_fast64_t;
typedef unsigned long uint_fast32_t;

/*******************************************************************************
 * Pseudo random number generator
 ******************************************************************************/
static unsigned long long mul64(unsigned long long x, unsigned long long y)
{
        unsigned long a, b, c, d, e, f, g, h;
        unsigned long long res = 0;
        a = x & 0xffff;
        x >>= 16;
        b = x & 0xffff;
        x >>= 16;
        c = x & 0xffff;
        x >>= 16;
        d = x & 0xffff;
        e = y & 0xffff;
        y >>= 16;
        f = y & 0xffff;
        y >>= 16;
        g = y & 0xffff;
        y >>= 16;
        h = y & 0xffff;
        res = d * e;
        res += c * f;
        res += b * g;
        res += a * h;
        res <<= 16;
        res += c * e;
        res += b * f;
        res += a * g;
        res <<= 16;
        res += b * e;
        res += a * f;
        res <<= 16;
        res += a * e;
        return res;
}

static const uint_fast64_t _multiplier = 0x5DEECE66DULL;
static const uint_fast64_t _addend = 0xB;
static const uint_fast64_t _mask = (1ULL << 48) - 1;
static uint_fast64_t _seed = 1;

// Assume that 1 <= _bits <= 32
static uint_fast32_t randBits(int _bits)
{
        uint_fast32_t rbits;
        uint_fast64_t nextseed = (mul64(_seed, _multiplier) + _addend) & _mask;
        _seed = nextseed;
        rbits = nextseed >> 16;
        return rbits >> (32 - _bits);
}

unsigned long rand()
{
        return randBits(32);
}

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


// Test 7

// *******************************************************************************
//  * Test 7
//  *
//  * Test de l'horloge (ARR et ACE)
//  * Tentative de determination de la frequence du processeur et de la
//  * periode de scheduling
//  ******************************************************************************


int sleep_pr1(void *arg)
{
        (void)arg;
        wait_clock(current_clock() + 2);
        printf(" not killed !!!");
        assert(0);
        return 1;
}

int timer_function(void *arg)
{
        volatile unsigned long *timer = NULL;
        timer = shm_acquire("test7_shm");
        assert(timer != NULL);

        (void)arg;
        while (1) {
                unsigned long t = *timer + 1;
                *timer = t;
                while (*timer == t) test_it();
        }
        while (1);
        return 0;
}

int timer1(void *arg)
{
        (void)arg;

        unsigned long quartz;
        unsigned long ticks;
        unsigned long dur;
        int i;

        clock_settings(&quartz, &ticks);
        dur = (quartz + ticks) / ticks;
        printf(" 2");
        for (i = 4; i < 8; i++) {
                wait_clock(current_clock() + dur);
                printf(" %d", i);
        }
        return 0;
}


#ifdef TELECOM_TST
int test7(void *arg)
{
        (void)arg;
        printf("Test desactive pour les TELECOM.\n");
}

#else

int test7(void *arg)
{
        int pid1, pid2, r;
        unsigned long c0, c, quartz, ticks, dur;
        volatile unsigned long *timer = NULL;

        (void)arg;
        timer = shm_create("test7_shm");
        assert(timer != NULL);

        assert(getprio(getpid()) == 128);
        printf("1");
        pid1 = start(timer1, 4000, 129, "timer1", 0);
        assert(pid1 > 0);
        printf(" 3");
        assert(waitpid(-1, 0) == pid1);
        printf(" 8 : ");

        *timer = 0;
        pid1 = start(timer_function, 4000, 127, "timer_function", 0);
        pid2 = start(timer_function, 4000, 127, "timer_function", 0);
        assert(pid1 > 0);
        assert(pid2 > 0);
        clock_settings(&quartz, &ticks);
        dur = 2 * quartz / ticks;
        test_it();
        c0 = current_clock();
        do {
                test_it();
                c = current_clock();
        } while (c == c0);
        wait_clock(c + dur);
        assert(kill(pid1) == 0);
        assert(waitpid(pid1, 0) == pid1);
        assert(kill(pid2) == 0);
        assert(waitpid(pid2, 0) == pid2);
        printf("%lu changements de contexte sur %lu tops d'horloge", *timer, dur);
        pid1 = start(sleep_pr1, 4000, 192,  "sleep_pr1", 0);
        assert(pid1 > 0);
        assert(kill(pid1) == 0);
        assert(waitpid(pid1, &r) == pid1);
        assert(r == 0);
        printf(".\n");
        shm_release("test7_shm");
        return 0;
}
#endif

//Test 8

/*******************************************************************************
 * Test 8
 *
 * Creation de processus se suicidant en boucle. Test de la vitesse de creation
 * de processus.
 ******************************************************************************/

int suicide(void *arg)
{
        (void)arg;
        kill(getpid());
        assert(0);
        return 0;
}

int suicide_launcher(void *arg)
{
	int pid1;
        (void)arg;
	pid1 = start(suicide, 4000, 192, "suicide", 0);
	assert(pid1 > 0);
	return pid1;
}

int test8(void *arg)
{
        unsigned long long tsc1;
        unsigned long long tsc2;
        int i, r, pid, count;

        (void)arg;
        assert(getprio(getpid()) == 128);

        /* Le petit-fils va passer zombie avant le fils mais ne pas
           etre attendu par waitpid. Un nettoyage automatique doit etre
           fait. */
        pid = start(suicide_launcher, 4000, 129, "suicide_launcher",0);
        assert(pid > 0);
        assert(waitpid(pid, &r) == pid);
        assert(chprio(r, 192) < 0);

        count = 0;
        __asm__ __volatile__("rdtsc":"=A"(tsc1));
        do {
                for (i=0; i<10; i++) {
                        pid = start(suicide_launcher, 4000, 200 , "suicide_launcher",0);
                        assert(pid > 0);
                        assert(waitpid(pid, 0) == pid);
                }
                test_it();
                count += i;
                __asm__ __volatile__("rdtsc":"=A"(tsc2));
        } while ((tsc2 - tsc1) < 1000000000);
        printf("%lu cycles/process.\n", (unsigned long)div64(tsc2 - tsc1, 2 * (unsigned)count));
        return 0;
}


/*******************************************************************************
 * Test 10
 * Test l'utilisation des semaphores ou des files de messages selon le sujet.
 *******************************************************************************/

#if defined WITH_SEM
/* Test d'utilisation d'un semaphore comme simple compteur. */
int main(void *arg)
{
        int sem1;
        (void)arg;
        sem1 = screate(2);
        assert(sem1 >= 0);
        assert(scount(sem1) == 2);
        assert(signal(sem1) == 0);
        assert(scount(sem1) == 3);
        assert(signaln(sem1, 2) == 0);
        assert(scount(sem1) == 5);
        assert(wait(sem1) == 0);
        assert(scount(sem1) == 4);
        assert(sreset(sem1, 7) == 0);
        assert(scount(sem1) == 7);
        assert(sdelete(sem1) == 0);
        printf("ok.\n");
        return 0;
}

#elif defined WITH_MSG
/* Test d'utilisation d'une file comme espace de stockage temporaire. */

static void write(int fid, const char *buf, unsigned long len)
{
        unsigned long i;
        for (i=0; i<len; i++) {
                assert(psend(fid, buf[i]) == 0);
        }
}

static void read(int fid, char *buf, unsigned long len)
{
        unsigned long i;
        for (i=0; i<len; i++) {
                int msg;
                assert(preceive(fid, &msg) == 0);
                buf[i] = (char)msg;
        }
}
int test10(void *arg)
{
        int fid;
        const char *str = "abcde";
        unsigned long len = strlen(str);
        char buf[10];

        (void)arg;

        printf("1");
        assert((fid = pcreate(5)) >= 0);
        write(fid, str, len);
        printf(" 2");
        read(fid, buf, len);
        buf[len] = 0;
        assert(strcmp(str, buf) == 0);
        assert(pdelete(fid) == 0);
        printf(" 3.\n");
        return 0;
}

#else
# error "WITH_SEM" ou "WITH_MSG" doit être définie.
#endif

//nothing.c
// int nothing(void *arg)
// {
//         (void)arg;
//         return 0;
// }

extern int nothing(void *arg);

//test_eax.c
static unsigned eax = 0xBADB00B5;
static unsigned *it_ok_test_eax = NULL;

void __test_valid_eax(unsigned a1)
{
        __asm__ __volatile__(
        "   pushl %%eax                 \n" /* Sauver %eax */
        "   movl  %1,       %%eax       \n" /* Attendre un scheduling, ie que *it_ok vaille 1 */
        "0: testl $1,       %2          \n"
        "   jz    0b                    \n"
        "   movl  %%eax,    %0          \n" /* Récupérer la valeur d'%eax après interruption */
        "   popl  %%eax                 \n" /* Restaurer %eax */
        : "=m" (eax)
        : "m" (a1), "m" (*it_ok_test_eax)
        : "%eax", "memory"
        );
        /* %eax doit avoir conservé sa valeur avant interruption ! */
        assert(eax == a1);
}

int test_eax(void *arg)
{
        (void)arg;
        it_ok_test_eax = (unsigned*) shm_acquire("test9_shm");
        *it_ok_test_eax = 0x0u;
        __test_valid_eax(rand());
        shm_release("test9_shm");
        return 0;

}

//test_reg2.c

static unsigned ebx = 1;
static unsigned ecx = 2;
static unsigned edx = 3;
static unsigned edi = 4;
static unsigned esi = 5;
static unsigned ebp_before = 0x12345678u;
static unsigned esp_before = 0x98765432u;
static unsigned ebp_after = 0xCAFE0101u;
static unsigned esp_after = 0x1010CAFEu;
static volatile unsigned *it_ok = NULL;

void __test_valid_regs2(unsigned a1,
                        unsigned a2,
                        unsigned a3,
                        unsigned a4,
                        unsigned a5)
{
        /* Initialise les registres avec des valeurs spéciales */
        __asm__ __volatile__(
        "movl %2,       %%ebx    \n"
        "movl %3,       %%ecx    \n"
        "movl %4,       %%edx    \n"
        "movl %5,       %%edi    \n"
        "movl %6,       %%esi    \n"
        /* Sauve le "stack pointer" et le "frame pointer" */
        "movl %%ebp,     %0      \n"
        "movl %%esp,     %1      \n"
        : "=m" (ebp_before), "=m" (esp_before)
        : "m" (a1), "m" (a2), "m" (a3), "m" (a4), "m" (a5)
        : "%ebx","%ecx", "%edx", "%edi", "%esi", "memory"
        );

        /* Attendre au moins un scheduling (ie des interruptions) */
        while (*it_ok == 0);

        /* Sauver les valeurs des registres */
        __asm__ __volatile__(
        "movl %%ebp,    %0      \n"
        "movl %%esp,    %1      \n"
        "movl %%ebx,    %2      \n"
        "movl %%ecx,    %3      \n"
        "movl %%edx,    %4      \n"
        "movl %%edi,    %5      \n"
        "movl %%esi,    %6      \n"
        : "=m" (ebp_after), "=m" (esp_after), "=m" (ebx), "=m" (ecx), "=m" (edx), "=m" (edi), "=m" (esi)
        : /* No input registers */
        : "memory"
        );

        /* Controler la validite des registres sauves */
        assert(ebp_before == ebp_after);
        assert(esp_before == esp_after);
        assert(ebx == a1);
        assert(ecx == a2);
        assert(edx == a3);
        assert(edi == a4);
        assert(esi == a5);
}

int test_regs2(void *arg)
{
        (void)arg;
        it_ok = (unsigned*) shm_acquire("test9_shm");
        *it_ok = 0x0u;
        __test_valid_regs2(rand(), rand(), rand(), rand(), rand());
        shm_release("test9_shm");
        return 0;
}

//test9.c 

static unsigned int ebp_before2 = 0x12345678u;
static unsigned int esp_before2 = 0x87654321u;
static unsigned int ebp_after2  = 0xDEADBEEFu;
static unsigned int esp_after2  = 0xDEADFACEu;
static unsigned int eax2        = 0xBADB00B5u;
static unsigned int ebx2        = 0xF0F0F0F0u;
static unsigned int edi2        = 0x0F0F0F0Fu;
static unsigned int esi2        = 0xABCDEFABu;

__asm__(
"       .data                   \n"
"nothing:                       \n"
"       .string \"nothing\"     \n"
"       .previous               \n"
);

void __test_valid_regs1(unsigned a1, unsigned a2, unsigned a3)
{
        __asm__ __volatile__(
        /* Assigner des valeurs connues aux registres */
        "movl   %8,     %%ebx           \n"
        "movl   %9,     %%edi           \n"
        "movl   %10,    %%esi           \n"
        "movl   %%ebp,  %0              \n"
        "movl   %%esp,  %1              \n"

        /* Démarrer le processus "nothing" */
        "pushl  $0                      \n"
        "pushl  $192                    \n"
        "pushl  $4000                   \n"
        "pushl  $nothing                \n"
        "call   start                   \n"
        "addl   $16,    %%esp           \n"
        "movl   %%eax,  %2              \n"

        /* Sauver les registres */
        "movl %%ebx,    %3              \n"
        "movl %%edi,    %4              \n"
        "movl %%esi,    %5              \n"
        "movl %%ebp,    %6              \n"
        "movl %%esp,    %7              \n"

        /* Registres de sortie */
        : "=m" (ebp_before2), "=m" (esp_before2),
          "=m" (eax2),        "=m" (ebx2),
          "=m" (edi2),        "=m" (esi2),
          "=m" (ebp_after2),  "=m" (esp_after2)
        /* Registres en entrée */
        : "m" (a1),
          "m" (a2),
          "m" (a3)
        /* Registres utilisés par ce bloc ASM*/
        : "eax", "ebx", "edi", "esi", "memory"
        );

        /* On attend le processus nothing dont le pid est dans eax */
        assert(waitpid((int)eax, NULL) == (int)eax);

        /* Vérifier les valeurs des registres après l'appel */
        assert(ebx2 == a1);
        assert(edi2 == a2);
        assert(esi2 == a3);
        assert(ebp_before2 == ebp_after2);
        assert(esp_before2 == esp_after2);
}

int test9(void *arg)
{
        int i;
        int pid;
        volatile unsigned *it_ok2 = NULL;

        (void)arg;
        it_ok2 = (unsigned*) shm_create("test9_shm");
        assert(it_ok2 != NULL);
        assert(getprio(getpid()) == 128);
        printf("1");

        for (i = 0; i < 1000; i++) {
                __test_valid_regs1(rand(), rand(), rand());
        }
        printf(" 2");

        /* Test de la cohérence de tous les registres */
        for (i = 0; i < 100; i++) {
                *it_ok2 = 1;
                pid = start(test_regs2, 4000, 128, "test_regs2", 0);
                assert(pid > 0);
                while (*it_ok2 != 0);
                *it_ok2 = 1;
                assert(waitpid(pid, 0) == pid);
        }
        printf(" 3");

        /* Test de la cohérence de %eax */
        for (i = 0; i < 100; i++) {
                *it_ok2 = 1;
                pid = start(test_eax, 4000, 128, "test_eax", 0);
                assert(pid > 0);
                while (*it_ok2 != 0);
                *it_ok2 = 1;
                assert(waitpid(pid, 0) == pid);
        }
        printf(" 4.\n");

        shm_release("test9_shm");
        return 0;
}

/*******************************************************************************
 * Test 11
 * Semaphore and mutex
 *******************************************************************************

int proc_mutex(void *arg)
{
        struct test11_shared *shared = NULL;
        int p = -1;
        int msg;

        (void)arg;
        shared = (struct test11_shared*) shm_acquire("test11_shm");
        assert(shared != NULL);
        p = getprio(getpid());
        assert(p > 0);

        switch (p) {
                case 130:
                        msg = 2;
                        break;
                case 132:
                        msg = 3;
                        break;
                case 131:
                        msg = 4;
                        break;
                case 129:
                        msg = 5;
                        break;
                default:
                        msg = 15;
        }
        printf(" %d", msg);
        xwait(&shared->sem);
        printf(" %d", 139 - p);
        assert(!(shared->in_mutex++));
        chprio(getpid(), 16);
        chprio(getpid(), p);
        shared->in_mutex--;
        xsignal(&shared->sem);
        return 0;

}
//Implémentation de verrous en se basant sur les sémaphores. 
void xwait(union sem *s)
{
        assert(wait(s->sem) == 0);
}

void xsignal(union sem *s)
{
        assert(signal(s->sem) == 0);
}

void xscreate(union sem *s)
{
        assert((s->sem = screate(0)) >= 0);
}

void xsdelete(union sem *s)
{
        assert(sdelete(s->sem) == 0);
}

int test11(void *arg)
{
        struct test11_shared *shared = NULL;
        int pid1, pid2, pid3, pid4;

        (void)arg;
        shared = (struct test11_shared*) shm_create("test11_shm");
        assert(shared != NULL);
        assert(getprio(getpid()) == 128);
        xscreate(&shared->sem);
        shared->in_mutex = 0;
        printf("1");
        pid1 = start(proc_mutex, 4000, 130, "proc_mutex", 0);
        pid2 = start(proc_mutex, 4000, 132, "proc_mutex", 0);
        pid3 = start(proc_mutex, 4000, 131, "proc_mutex", 0);
        pid4 = start(proc_mutex, 4000, 129, "proc_mutex", 0);
        assert(pid1 > 0);
        assert(pid2 > 0);
        assert(pid3 > 0);
        assert(pid4 > 0);
        assert(chprio(getpid(), 160) == 128);
        printf(" 6");
        xsignal(&shared->sem);
        assert(waitpid(-1, 0) == pid2);
        assert(waitpid(-1, 0) == pid3);
        assert(waitpid(-1, 0) == pid1);
        assert(waitpid(-1, 0) == pid4);
        assert(waitpid(-1, 0) < 0);
        assert(chprio(getpid(), 128) == 160);
        xsdelete(&shared->sem);
        printf(" 11.\n");

        return 0;
}*/


/*******************************************************************************
 * Test 12
 *
 * Tests de rendez-vous sur une file de taille 1.
 ******************************************************************************/

int rdv_proc(void *arg)
{
        int fid = (int) arg;
        int msg;
        int count;

        printf(" 2");
        assert(psend(fid, 3) == 0); /* Depose dans le tampon */
        printf(" 3");
        assert((pcount(fid, &count) == 0) && (count == 1));
        assert(psend(fid, 4) == 0); /* Bloque tampon plein */
        printf(" 5");
        assert((pcount(fid, &count) == 0) && (count == 1));
        assert(preceive(fid, &msg) == 0); /* Retire du tampon */
        assert(msg == 4);
        printf(" 6");
        assert(preceive(fid, &msg) == 0); /* Bloque tampon vide. */
        assert(msg == 5);
        printf(" 8");
        assert((pcount(fid, &count) == 0) && (count == 0));
        return 0;
}



int test12(void *arg)
{
        int fid;
        int pid;
        int msg;
        int count;

        (void)arg;

        assert(getprio(getpid()) == 128);
        assert((fid = pcreate(1)) >= 0);
        printf("1");
        pid = start(rdv_proc, 4000, 130, "rdv_proc",(void *)fid);
        assert(pid > 0);
        printf(" 4");
        assert((pcount(fid, &count) == 0) && (count == 2));
        assert(preceive(fid, &msg) == 0); /* Retire du tampon et debloque un emetteur. */
        assert(msg == 3);
        printf(" 7");
        assert((pcount(fid, &count) == 0) && (count == -1));
        assert(psend(fid, 5) == 0); /* Pose dans le tampon. */
        printf(" 9");
        assert(psend(fid, 6) == 0); /* Pose dans le tampon. */
        assert(preceive(fid, &msg) == 0); /* Retire du tampon. */
        assert(msg == 6);
        assert(pdelete(fid) == 0);
        assert(psend(fid, 2) < 0);
        assert(preceive(fid, &msg) < 0);
        assert(waitpid(-1, 0) == pid);
        printf(" 10.\n");
        return 0;
}

/*******************************************************************************
 * Test 13
 *
 * Teste l'ordre entre les processus emetteurs et recepteurs sur une file.
 * Teste le changement de priorite d'un processus bloque sur une file.
 ******************************************************************************/

int preceiver(void *arg)
{
        struct psender *ps = NULL;
        int ps_index = (int)arg;
        int msg;
        unsigned i;
        unsigned n;

        ps = shm_acquire("test13_shm");
        assert(ps != NULL);
        n = strlen(ps[ps_index].data);

        for(i = 0; i < n; i++) {
                assert(preceive(ps[ps_index].fid, &msg) == 0);
                assert(msg == ps[ps_index].data[i]);
        }

        shm_release("test13_shm");
        return 0;
}

int psender(void *arg)
{
        struct psender *ps = NULL;
        int ps_index = (int)arg;
        unsigned i;
        unsigned n;

        ps = shm_acquire("test13_shm");
        assert(ps != NULL);
        n = strlen(ps[ps_index].data);

        for(i = 0; i < n; i++) {
                assert(psend(ps[ps_index].fid, ps[ps_index].data[i]) == 0);
        }
        shm_release("test13_shm");
        return 0;
}


int test13(void *arg)
{
        struct psender *ps = NULL;
        int pid1, pid2, pid3;
        int fid = pcreate(3);
        int i, msg;

        (void)arg;
        ps = (struct psender*) shm_create("test13_shm");
        assert(ps != NULL);

        printf("1");
        assert(getprio(getpid()) == 128);
        assert(fid >= 0);
        ps[1].fid = ps[2].fid = ps[3].fid = fid;
        strncpy(ps[1].data, "abcdehm", 32);
        strncpy(ps[2].data, "il", 32);
        strncpy(ps[3].data, "fgjk", 32);
        pid1 = start(psender, 4000, 131, "psender",(void*)1);
        pid2 = start(psender, 4000, 130,"psender", (void*)2);
        pid3 = start(psender, 4000, 129, "psender",(void*)3);
        for (i=0; i<2; i++) {
                assert(preceive(fid, &msg) == 0);
                assert(msg == 'a' + i);
        }
        chprio(pid1, 129);
        chprio(pid3, 131);
        for (i=0; i<2; i++) {
                assert(preceive(fid, &msg) == 0);
                assert(msg == 'c' + i);
        }
        chprio(pid1, 127);
        chprio(pid2, 126);
        chprio(pid3, 125);
        for (i=0; i<6; i++) {
                assert(preceive(fid, &msg) == 0);
                assert(msg == 'e' + i);
        }
        chprio(pid1, 125);
        chprio(pid3, 127);
        for (i=0; i<3; i++) {
                assert(preceive(fid, &msg) == 0);
                assert(msg == 'k' + i);
        }
        assert(waitpid(pid3, 0) == pid3); //XXX assert(waitpid(-1, 0) == pid3); ???
        assert(waitpid(-1, 0) == pid2);
        assert(waitpid(-1, 0) == pid1);
        printf(" 2");

        strncpy(ps[1].data, "abej", 32);
        strncpy(ps[2].data, "fi", 32);
        strncpy(ps[3].data, "cdgh", 32);
        pid1 = start(preceiver, 4000, 131, "preceiver",(void*)1);
        pid2 = start(preceiver, 4000, 130, "preceiver",(void*)2);
        pid3 = start(preceiver, 4000, 129, "preceiver",(void*)3);
        for (i='a'; i<='b'; i++) {
                assert(psend(fid, i) == 0);
        }
        chprio(pid1, 129);
        chprio(pid3, 131);
        for (i='c'; i<='d'; i++) {
                assert(psend(fid, i) == 0);
        }
        chprio(pid1, 127);
        chprio(pid2, 126);
        chprio(pid3, 125);
        for (i='e'; i<='j'; i++) {
                assert(psend(fid, i) == 0);
        }
        chprio(pid1, 125);
        chprio(pid3, 127);
        assert(waitpid(-1, 0) == pid3);
        assert(waitpid(-1, 0) == pid2);
        assert(waitpid(-1, 0) == pid1);
        assert(pdelete(fid) == 0);
        printf(" 3.\n");
        shm_release("test13_shm");
        return 0;
}


/*******************************************************************************
 * Test 14
 *
 * Tests de preset et pdelete
 ******************************************************************************/

int psender1(void *arg)
{
        int fid1 = (int)arg;
        int fid2;
        int msg;

        printf(" 2");
        assert(preceive(fid1, &fid2) == 0);
        assert(psend(fid1, fid2) == 0);
        fid2 -= 42;
        assert(psend(fid1, 1) == 0);
        assert(psend(fid1, 2) == 0);
        assert(psend(fid1, 3) == 0);
        assert(psend(fid1, 4) == 0);
        assert(psend(fid1, 5) < 0);
        printf(" 6");
        assert(psend(fid1, 12) < 0);
        printf(" 9");
        assert(psend(fid1, 14) < 0);
        assert(preceive(fid2, &msg) < 0);
        printf(" 12");
        assert(preceive(fid2, &msg) < 0);
        assert(preceive(fid2, &msg) < 0);
        return 0;
}


int psender2(void *arg)
{
        int fid1 = (int)arg;
        int fid2;
        int msg;

        printf(" 3");
        assert(preceive(fid1, &fid2) == 0);
        fid2 -= 42;
        assert(psend(fid1, 6) < 0);
        printf(" 5");
        assert(psend(fid1, 7) == 0);
        assert(psend(fid1, 8) == 0);
        assert(psend(fid1, 9) == 0);
        assert(psend(fid1, 10) == 0);
        assert(psend(fid1, 11) < 0);
        printf(" 8");
        assert(psend(fid1, 13) < 0);
        assert((preceive(fid2, &msg) == 0) && (msg == 15));
        assert(preceive(fid2, &msg) < 0);
        printf(" 11");
        assert(preceive(fid2, &msg) < 0);
        assert(preceive(fid2, &msg) < 0);
        return 0;
}


int test14(void *arg)
{
        int pid1, pid2;
        int fid1 = pcreate(3);
        int fid2 = pcreate(3);
        int msg;

        (void)arg;

        /* Bravo si vous n'etes pas tombe dans le piege. */
        assert(pcreate(1073741827) < 0);

        printf("1");
        assert(getprio(getpid()) == 128);
        assert(fid1 >= 0);
        assert(psend(fid1, fid2 + 42) == 0);
        pid1 = start(psender1, 4000, 131, "psender1",(void *)fid1);
        pid2 = start(psender2, 4000, 130, "psender2",(void *)fid1);
        assert((preceive(fid1, &msg) == 0) && (msg == 1));
        assert(chprio(pid2, 132) == 130);
        printf(" 4");
        assert(preset(fid1) == 0);
        assert((preceive(fid1, &msg) == 0) && (msg == 7));
        printf(" 7");
        assert(pdelete(fid1) == 0);
        printf(" 10");
        assert(psend(fid2, 15) == 0);
        assert(preset(fid2) == 0);
        printf(" 13");
        assert(pdelete(fid2) == 0);
        assert(pdelete(fid2) < 0);
        assert(waitpid(pid2, 0) == pid2); //XXX assert(waitpid(-1, 0) == pid2); ???
        assert(waitpid(-1, 0) == pid1);
        printf(".\n");
        return 1;
}







#define NB_TEST_CASE 15
static int size = NB_TEST_CASE;                 /*Mark null to not execute the test case*/
static int (*test_case[NB_TEST_CASE])(void *) = {test0, test1, test2, test3, test4, test5, test6, NULL, test8, NULL, test10, NULL, test12,test13,test14};

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
