#include <inc/lib.h>

void go(void*para)
{
	cprintf("hello world! from thread %d\n", sys_getthdid());
}

void umain(int argc, char **argv)
{
	thdid_t r = create_thread(go, NULL);
	wait_thread(r);
	cprintf("hello world! from thread %d\n", sys_getthdid());
}
