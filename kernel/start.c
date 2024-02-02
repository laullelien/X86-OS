#include "debugger.h"
#include "cpu.h"
#include "printf.c"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}


void kernel_start(void)
{
	int i;
//	call_debugger();

	printf("test\ntoto");
	i = 10;

	i = fact(i);

	while(1)
	  hlt();

	return;
}
