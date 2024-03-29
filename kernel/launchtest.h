/*
 *  2024 --x86-deloryau-dussuds-rebouluc-regnierl-wyckaese
 *
 *   Manager who organize the launch of the tests
 */

#ifndef __LAUNCHTEST_H__
#define __LAUNCHTEST_H__

//Const for tests:
#define DUMMY_VAL 78

#define WITH_MSG 1

//Test 4 
#ifdef microblaze
static const int loop_count0 = 500;
static const int loop_count1 = 1000;
#else
static const int loop_count0 = 5000;
static const int loop_count1 = 10000;
#endif

/* Wrapper sur les verrous basés sur les sémaphores ou les files de messages */
union sem {
    int fid;
    int sem;
};

struct test11_shared {
        union sem sem;
        int in_mutex;
};

struct psender {
    int fid;
    char data[32];
};



extern int launchtest();

#endif
