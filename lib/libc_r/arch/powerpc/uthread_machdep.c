/*	$OpenBSD: uthread_machdep.c,v 1.1 2000/09/25 01:16:40 d Exp $	*/
/* David Leonard, <d@csee.uq.edu.au>. Public domain */

#include <pthread.h>
#include "pthread_private.h"

#define ALIGNBYTES	0xf

/* Register save frame as it appears on the stack */
struct _machdep_frame {
	int     r1;
	int     reserved;
	int     gp[32-14];
	int     lr, cr, ctr, xer;
	long    fp[32-14];
	long    fs;
	/* The rest are only valid in the initial frame */
	int     next_r1;
	int     next_lr;
};

/*
 * Given a stack and an entry function, initialise a state
 * structure that can be later switched to.
 */
void
_thread_machdep_init(statep, base, len, entry)
	struct _machdep_state* statep;
	void *base;
	int len;
	void (*entry)(void);
{
	struct _machdep_frame *f;

	/* Locate the initial frame, aligned at the top of the stack */
	f = (struct _machdep_frame *)(((int)base + len - sizeof *f) & ~ALIGNBYTES);
	
	f->r1 = (int)&f->next_r1;
	f->reserved = 0;
	f->lr = (int)entry;
	f->next_r1 = 0;		/* for gdb */
	f->next_lr = 0;		/* for gdb */

	/* Initialise the new thread with all the state from this thread. */

#define copyreg(x) __asm__("stw " #x ", %0" : "=m"(f->gp[x-14]))
	copyreg(14); copyreg(15); copyreg(16); copyreg(17); copyreg(18);
	copyreg(19); copyreg(20); copyreg(21); copyreg(22); copyreg(23);
	copyreg(24); copyreg(25); copyreg(26); copyreg(27); copyreg(28);
	copyreg(29); copyreg(30); copyreg(31);

#define copysreg(nm) __asm__("mf" #nm " %0" : "=r"(f->nm))
	copysreg(cr); copysreg(ctr); copysreg(xer);

#define copyfreg(x) __asm__("stfd " #x ", %0" : "=m"(f->fp[x-14]))
	copyfreg(14); copyfreg(15); copyfreg(16); copyfreg(17); copyfreg(18);
	copyfreg(19); copyfreg(20); copyfreg(21); copyfreg(22); copyfreg(23);
	copyfreg(24); copyfreg(25); copyfreg(26); copyfreg(27); copyfreg(28);
	copyfreg(29); copyfreg(30); copyfreg(31);

	__asm__("mffs 0; stfd 0, %0" : "=m"(f->fs));

	statep->frame = (int)f;
}


/*
 * No-op float saves.
 * (Floating point registers were saved in _thread_machdep_switch())
 */

void
_thread_machdep_save_float_state(statep)
	struct _machdep_state* statep;
{
}

void
_thread_machdep_restore_float_state(statep)
	struct _machdep_state* statep;
{
}
