
#include <inc/lib.h>

#define THREAD_MAX (PTSIZE / 4)

volatile int thread_used_stack[THREAD_MAX];
volatile uint32_t t_lock = 0;

static int get_empty_thread_position();

static __inline void __attribute__((always_inline))
thread_lock()
{
	while(xchg(&t_lock, 1))
		sys_yield();
}

static __inline void __attribute__((always_inline))
thread_unlock()
{
	if (xchg(&t_lock, 0) == 0)
		panic(" thread_unlock: bad lock");
}

void
wait_thread(thdid_t tar)
{
	int p = THDX(tar);
	//cprintf("wait for %d\n", p);
	while(thds[p].thd_status != THD_FREE && thds[p].thd_id == tar)
		sys_yield();
}

// never call this function directly!
static void
thread_start(void(*func)(void*), void *para)
{
	//cprintf("lib: new_thread func=%p para=%p\n",func, para);
	func(para);
	//cprintf("lib: delete_thread\n");
	delete_thread(0);
}

thdid_t
create_thread(void(*func)(void*), void*para)
{
	thdid_t child = sys_thd_create();
	if (child < 0)
		return child;
	thread_lock();
	int p = get_empty_thread_position();
	if (p < 0)
		panic(" create_thread error 0: Do not have thread space for new thread!");
	thread_used_stack[p] = child;
	thread_unlock();
	int r = sys_page_alloc(0, (void*)(UTXSTACKTOP(p) - PGSIZE), PTE_P | PTE_U | PTE_W);
	if (r < 0)
		panic(" create_thread error 1: %e", r);
	r = sys_page_alloc(0, (void*)(UTSTACKTOP(p) - PGSIZE), PTE_P | PTE_U | PTE_W);
	if (r < 0)
		panic(" create_thread error 2: %e", r);

	// set stack for new thread
	uintptr_t *tmp = (uintptr_t*)UTSTACKTOP(p);
	tmp -= 2;
	*(--tmp) = (unsigned)para;
	*(--tmp) = (unsigned)func;

	struct Trapframe tf;
	memset(&tf, 0, sizeof tf);
	tf.tf_eflags = 0;
	tf.tf_eip = (uintptr_t)thread_start;
	tf.tf_esp = (uintptr_t)(tmp - 1);
	r = sys_thd_set_trapframe(child, &tf);
	if (r < 0)
		panic(" create_thread error 3: %e", r);
	r = sys_thd_set_uxstack(child, UTXSTACKTOP(p) - PGSIZE);
	if (r < 0)
		panic(" create_thread error 4: %e", r);
	r = sys_thd_set_status(child, THD_RUNNABLE);
	if (r < 0)
		panic(" create_thread error 5: %e", r);
	return child;
}

int
delete_thread(thdid_t tar)
{
	thdid_t cur = sys_getthdid();
	if (tar == 0)
		tar = cur;
	if (tar == main_thdid)
		return -E_INVAL;
	int stk;
	for(stk = 1; stk < THREAD_MAX; stk++)
		if (thread_used_stack[stk] == tar)
			break;
	if (cur == tar)
	{
		if (stk == THREAD_MAX)
			panic("delete_thread error 0: target thread does not have stack");
		// cannot use stack now

		// it seems that we do not have to lock here, LiChao
		thread_lock();
		thread_used_stack[stk] = 0;
		//thread_unlock();
		asm volatile("lock; xchgl %0, %1" :
				"+m" (t_lock) :
				"a" (0) :
				"cc");

		// sys_thd_destroy(0);
		asm volatile("int %0\n"
					:
					  : "i" (T_SYSCALL),
					    "a" (SYS_thd_destroy),
					    "d" (0),
					    "c" (0),
					    "b" (0),
					    "D" (0),
					    "S" (0)
				: "cc", "memory");
		// cannot reach here
		assert(0);
	} else
	{
		int r = sys_thd_destroy(tar);
		if (r < 0)
			return r;
		if (stk == THREAD_MAX)
			panic("delete_thread error 2: target thread does not have stack");
		wait_thread(tar);
		thread_lock();
		thread_used_stack[stk] = 0;
		thread_unlock();
	}
	return 0;
}

static int
get_empty_thread_position()
{
	int i;
	for(i = 1; i < THREAD_MAX; i++)
		if (thread_used_stack[i] == 0)
			return i;
	return -1;
}

