// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	int perm = 0xfff & uvpt[PGNUM(addr)];
	if (!((err & FEC_WR) && (perm & PTE_COW) && (perm & PTE_P) && (uvpd[PDX(addr)] & PTE_P)))
		panic("not a write or a cow page");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	int tmp = sys_page_alloc(0, (void*)PFTEMP, PTE_U | PTE_P | PTE_W);
	if (tmp < 0)
		panic("At pgfault 1: %e",tmp);
	addr = ROUNDDOWN(addr, PGSIZE);
	memmove((void*)PFTEMP, addr, PGSIZE);
	tmp = sys_page_map(0, (void*)PFTEMP, 0, addr, PTE_U | PTE_P | PTE_W);
	if (tmp < 0)
		panic("At pgfault 2: %e",tmp);
	tmp = sys_page_unmap(0, (void*)PFTEMP);
	if (tmp < 0)
		panic("At pgfault 3: %e",tmp);

	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	// LAB 4: Your code here.
	pn <<= PGSHIFT;
	if (!(uvpd[PDX(pn)] & PTE_P))
		return 0;
	int perm = 0xfff & uvpt[PGNUM(pn)];
	if (!(perm & PTE_P))
		return 0;
	if ((perm & PTE_U) && ((perm & PTE_W) || (perm & PTE_COW)))
	{
		int tmp = sys_page_map(0, (void*)pn, envid, (void*)pn, PTE_COW | PTE_U | PTE_P);
		if (tmp < 0)
			panic("At duppage 1: %e", tmp);
		tmp = sys_page_map(0, (void*)pn, 0, (void*)pn, PTE_COW | PTE_P | PTE_U);
		if (tmp < 0)
			panic("At duppage 2: %e", tmp);
	} else
	{
		int tmp = sys_page_map(0, (void*)pn, envid, (void*)pn, perm);
		if (tmp < 0)
			panic("At duppage 3: %e", tmp);
	}
	//panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	envid_t ret = sys_exofork();
	if (ret == 0)
	{
		thisenv = envs + ENVX(sys_getenvid());
		return 0;
	}
	if (ret < 0)
		panic("At fork 1: %e", ret);
	uintptr_t i;
	for(i = 0; i < UTOP; i += PGSIZE) if (i != UXSTACKTOP - PGSIZE)
		duppage(ret, PGNUM(i));
	int tmp = sys_page_alloc(ret, (void*)(UXSTACKTOP - PGSIZE), PTE_U | PTE_W | PTE_P);
	if (tmp < 0)
		panic("At fork 2: %e", tmp);
	extern void _pgfault_upcall(void);
	tmp = sys_env_set_pgfault_upcall(ret, _pgfault_upcall);
	if (tmp < 0)
		panic("At fork 3: %e", tmp);
	tmp = sys_env_set_status(ret, ENV_RUNNABLE);
	if (tmp < 0)
		panic("At fork 4: %e", tmp);
	return ret;
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
