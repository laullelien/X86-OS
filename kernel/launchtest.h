/*
 *  2024 --x86-deloryau-dussuds-rebouluc-regnierl-wyckaese
 *
 *   Manager who organize the launch of the tests
 */

#ifndef __LAUNCHTEST_H__
#define __LAUNCHTEST_H__

//Const for tests:
#define DUMMY_VAL 78

//Test 4 
#ifdef microblaze
static const int loop_count0 = 500;
static const int loop_count1 = 1000;
#else
static const int loop_count0 = 5000;
static const int loop_count1 = 10000;
#endif


extern int launchtest();

#endif
