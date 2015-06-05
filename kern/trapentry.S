/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(default_divide_error, T_DIVIDE)
TRAPHANDLER_NOEC(default_debug_exception, T_DEBUG)
TRAPHANDLER_NOEC(default_nmi_interrupt, T_NMI)
TRAPHANDLER_NOEC(default_breakpoint, T_BRKPT)
TRAPHANDLER_NOEC(default_overflow, T_OFLOW)
TRAPHANDLER_NOEC(default_bound_check, T_BOUND)
TRAPHANDLER_NOEC(default_illegal_opcode, T_ILLOP)
TRAPHANDLER_NOEC(default_device_not_available, T_DEVICE)
TRAPHANDLER(default_double_fault, T_DBLFLT)
/* RESERVED */
TRAPHANDLER(default_invalid_tss, T_TSS)
TRAPHANDLER(default_segment_not_present, T_SEGNP)
TRAPHANDLER(default_stack_exception, T_STACK)
TRAPHANDLER(default_general_protection_fault, T_GPFLT)
TRAPHANDLER(default_page_fault, T_PGFLT)
/* RESERVED */
TRAPHANDLER_NOEC(default_floating_point_error, T_FPERR)
TRAPHANDLER(default_alignment_check, T_ALIGN)
TRAPHANDLER_NOEC(default_machine_check, T_MCHK)
TRAPHANDLER_NOEC(default_simd_floating_point_error, T_SIMDERR)

TRAPHANDLER_NOEC(default_irq0, IRQ_OFFSET)
TRAPHANDLER_NOEC(default_irq1, IRQ_OFFSET + 1)
TRAPHANDLER_NOEC(default_irq2, IRQ_OFFSET + 2)
TRAPHANDLER_NOEC(default_irq3, IRQ_OFFSET + 3)
TRAPHANDLER_NOEC(default_irq4, IRQ_OFFSET + 4)
TRAPHANDLER_NOEC(default_irq5, IRQ_OFFSET + 5)
TRAPHANDLER_NOEC(default_irq6, IRQ_OFFSET + 6)
TRAPHANDLER_NOEC(default_irq7, IRQ_OFFSET + 7)
TRAPHANDLER_NOEC(default_irq8, IRQ_OFFSET + 8)
TRAPHANDLER_NOEC(default_irq9, IRQ_OFFSET + 9)
TRAPHANDLER_NOEC(default_irq10, IRQ_OFFSET + 10)
TRAPHANDLER_NOEC(default_irq11, IRQ_OFFSET + 11)
TRAPHANDLER_NOEC(default_irq12, IRQ_OFFSET + 12)
TRAPHANDLER_NOEC(default_irq13, IRQ_OFFSET + 13)
TRAPHANDLER_NOEC(default_irq14, IRQ_OFFSET + 14)
TRAPHANDLER_NOEC(default_irq15, IRQ_OFFSET + 15)

TRAPHANDLER_NOEC(default_system_call, T_SYSCALL)

/*
 * Lab 3: Your code here for _alltraps
 */

_alltraps:
	pushl %ds
	pushl %es
	pushal
	movl $GD_KD, %eax
	movw %ax, %ds
	movw %ax, %es
	pushl %esp
	call trap
