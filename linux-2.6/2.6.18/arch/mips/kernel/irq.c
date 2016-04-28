/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Code to handle x86 style IRQs plus some generic interrupt stuff.
 *
 * Copyright (C) 1992 Linus Torvalds
 * Copyright (C) 1994 - 2000 Ralf Baechle
 */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/kallsyms.h>

#include <asm/atomic.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/kgdb.h>

/* Keep track of if we've done certain initialization already or not. */
int kgdb_early_setup;

static unsigned long irq_map[NR_IRQS / BITS_PER_LONG];

int __devinit allocate_irqno(void)
{
	int irq;

again:
	irq = find_first_zero_bit(irq_map, NR_IRQS);

	if (irq >= NR_IRQS)
		return -ENOSPC;

	if (test_and_set_bit(irq, irq_map))
		goto again;

	return irq;
}

EXPORT_SYMBOL_GPL(allocate_irqno);

/*
 * Allocate the 16 legacy interrupts for i8259 devices.  This happens early
 * in the kernel initialization so treating allocation failure as BUG() is
 * ok.
 */
void __init alloc_legacy_irqno(void)
{
	int i;

	for (i = 0; i <= 16; i++)
		BUG_ON(test_and_set_bit(i, irq_map));
}

void __devinit free_irqno(unsigned int irq)
{
	smp_mb__before_clear_bit();
	clear_bit(irq, irq_map);
	smp_mb__after_clear_bit();
}

EXPORT_SYMBOL_GPL(free_irqno);

/*
 * 'what should we do if we get a hw irq event on an illegal vector'.
 * each architecture has to answer this themselves.
 */
void ack_bad_irq(unsigned int irq)
{
	printk("unexpected IRQ # %d\n", irq);
}

atomic_t irq_err_count;

#ifdef CONFIG_MIPS_MT_SMTC
/*
 * SMTC Kernel needs to manipulate low-level CPU interrupt mask
 * in do_IRQ. These are passed in setup_irq_smtc() and stored
 * in this table.
 */
unsigned long irq_hwmask[NR_IRQS];
#endif /* CONFIG_MIPS_MT_SMTC */

#undef do_IRQ

// Check the return address pointer located at offset o from the stack pointer.
// This needs to be tuned for a specific function prologue.
#define CHECK_RA(o)		{ int x;				\
		asm volatile (" lw %0," #o "($sp); nop; lw %0,0(%0);"	\
		    : "=r" (x) : ); }
		    
		    

/*
 * do_IRQ handles all normal device IRQ's (the special
 * SMP cross-CPU interrupts have their own specific
 * handlers).
 */
asmlinkage unsigned int do_IRQ(unsigned int irq, struct pt_regs *regs)
{
	CHECK_RA(16);
	irq_enter();
	CHECK_RA(16);

	__DO_IRQ_SMTC_HOOK(irq);
	__do_IRQ(irq, regs);
	CHECK_RA(16);

	irq_exit();
	CHECK_RA(16);

	return 1;
}

/*
 * Generic, controller-independent functions:
 */

int show_interrupts(struct seq_file *p, void *v)
{
	int i = *(loff_t *) v, j;
	struct irqaction * action;
	unsigned long flags;

	if (i == 0) {
		seq_printf(p, "           ");
		for_each_online_cpu(j)
			seq_printf(p, "CPU%d       ",j);
		seq_putc(p, '\n');
	}

	if (i < NR_IRQS) {
		spin_lock_irqsave(&irq_desc[i].lock, flags);
		action = irq_desc[i].action;
		if (!action)
			goto skip;
		seq_printf(p, "%3d: ",i);
#ifndef CONFIG_SMP
		seq_printf(p, "%10u ", kstat_irqs(i));
#else
		for_each_online_cpu(j)
			seq_printf(p, "%10u ", kstat_cpu(j).irqs[i]);
#endif
		seq_printf(p, " %14s", irq_desc[i].chip->typename);
		seq_printf(p, "  %s", action->name);

		for (action=action->next; action; action = action->next)
			seq_printf(p, ", %s", action->name);

		seq_putc(p, '\n');
skip:
		spin_unlock_irqrestore(&irq_desc[i].lock, flags);
	} else if (i == NR_IRQS) {
		seq_putc(p, '\n');
		seq_printf(p, "ERR: %10u\n", atomic_read(&irq_err_count));
	}
	return 0;
}

#ifdef CONFIG_TIVO_IRQ_DEBUG
/*
 * This function would achieve what we would have done
 * by reading the /proc/interrupts interface.
 */
void show_interrupts_on_die()
{
	int i = 0, j, num = 0;
	struct irqaction * action;
	unsigned long flags;

	printk ("/proc/interrupts: \n"); 
	printk ("           ");
	for_each_online_cpu(j)
		printk ("CPU%d       ",j);
	printk ("\n");

	while (i++ < NR_IRQS) {
		spin_lock_irqsave(&irq_desc[i].lock, flags);
		action = irq_desc[i].action;

		if (!action){
#ifndef CONFIG_SMP
			num = kstat_irqs(i);
#else
			for_each_online_cpu(j)
				num += kstat_cpu(j).irqs[i];
#endif
			if (num) {
				printk ("%3d: %10u Unhandled Interrupts\n", i, num);
				num = 0;
			}
			
			spin_unlock_irqrestore(&irq_desc[i].lock, flags);
			continue;
		}

		printk ("%3d: ",i);
#ifndef CONFIG_SMP
		printk ("%10u ", kstat_irqs(i));
#else
		for_each_online_cpu(j)
		printk ("%10u ", kstat_cpu(j).irqs[i]);
#endif
		printk (" %14s", irq_desc[i].chip->typename);
		printk ("  %s", action->name);

		for (action=action->next; action; action = action->next)
			printk (", %s", action->name);

		printk ("\n");
		spin_unlock_irqrestore(&irq_desc[i].lock, flags);
	} 
}
#endif

asmlinkage void spurious_interrupt(struct pt_regs *regs)
{
	atomic_inc(&irq_err_count);
}

void __init init_IRQ(void)
{
	int i;

	if (kgdb_early_setup)
		return;

	for (i = 0; i < NR_IRQS; i++) {
		irq_desc[i].status  = IRQ_DISABLED;
		irq_desc[i].action  = NULL;
		irq_desc[i].depth   = 1;
		irq_desc[i].chip = &no_irq_chip;
		spin_lock_init(&irq_desc[i].lock);
#ifdef CONFIG_MIPS_MT_SMTC
		irq_hwmask[i] = 0;
#endif /* CONFIG_MIPS_MT_SMTC */
	}

	arch_init_irq();
#ifdef CONFIG_KGDB
	/*
	 * We have been called before kgdb_arch_init(). Hence,
	 * we dont want the traps to be reinitialized
	 */
	if (kgdb_early_setup == 0)
		kgdb_early_setup = 1;
#endif
}