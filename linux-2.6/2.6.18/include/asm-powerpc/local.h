#ifndef _ARCH_POWERPC_LOCAL_H
#define _ARCH_POWERPC_LOCAL_H

#include <linux/percpu.h>
#include <asm/atomic.h>

typedef struct
{
	atomic_long_t a;
} local_t;

#define LOCAL_INIT(i)	{ ATOMIC_LONG_INIT(i) }

#define local_read(l)	atomic_long_read(&(l)->a)
#define local_set(l,i)	atomic_long_set(&(l)->a, (i))

#define local_add(i,l)	atomic_long_add((i),(&(l)->a))
#define local_sub(i,l)	atomic_long_sub((i),(&(l)->a))
#define local_inc(l)	atomic_long_inc(&(l)->a)
#define local_dec(l)	atomic_long_dec(&(l)->a)

#ifndef __powerpc64__

static __inline__ int local_add_return(int a, local_t *l)
{
	int t;

	__asm__ __volatile__(
"1:	lwarx	%0,0,%2		# local_add_return\n\
	add	%0,%1,%0\n"
	PPC405_ERR77(0,%2)
"	stwcx.	%0,0,%2 \n\
	bne-	1b"
	: "=&r" (t)
	: "r" (a), "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

#define local_add_negative(a, l)	(local_add_return((a), (l)) < 0)

static __inline__ int local_sub_return(int a, local_t *l)
{
	int t;

	__asm__ __volatile__(
"1:	lwarx	%0,0,%2		# local_sub_return\n\
	subf	%0,%1,%0\n"
	PPC405_ERR77(0,%2)
"	stwcx.	%0,0,%2 \n\
	bne-	1b"
	: "=&r" (t)
	: "r" (a), "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

static __inline__ int local_inc_return(local_t *l)
{
	int t;

	__asm__ __volatile__(
"1:	lwarx	%0,0,%1		# local_inc_return\n\
	addic	%0,%0,1\n"
	PPC405_ERR77(0,%1)
"	stwcx.	%0,0,%1 \n\
	bne-	1b"
	: "=&r" (t)
	: "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

/*
 * local_inc_and_test - increment and test
 * @l: pointer of type local_t
 *
 * Atomically increments @l by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
#define local_inc_and_test(l) (local_inc_return(l) == 0)

static __inline__ int local_dec_return(local_t *l)
{
	int t;

	__asm__ __volatile__(
"1:	lwarx	%0,0,%1		# local_dec_return\n\
	addic	%0,%0,-1\n"
	PPC405_ERR77(0,%1)
"	stwcx.	%0,0,%1\n\
	bne-	1b"
	: "=&r" (t)
	: "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

#define local_cmpxchg(l, o, n) \
	((long)cmpxchg(&((l)->a.counter), (o), (n)))
#define local_xchg(l, new) (xchg(&((l)->a.counter), new))

/**
 * local_add_unless - add unless the number is a given value
 * @l: pointer of type local_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @l, so long as it was not @u.
 * Returns non-zero if @l was not @u, and zero otherwise.
 */
static __inline__ int local_add_unless(local_t *l, int a, int u)
{
	int t;

	__asm__ __volatile__ (
"1:	lwarx	%0,0,%1		# local_add_unless\n\
	cmpw	0,%0,%3 \n\
	beq-	2f \n\
	add	%0,%2,%0 \n"
	PPC405_ERR77(0,%2)
"	stwcx.	%0,0,%1 \n\
	bne-	1b \n"
"	subf	%0,%2,%0 \n\
2:"
	: "=&r" (t)
	: "r" (&(l->a.counter)), "r" (a), "r" (u)
	: "cc", "memory");

	return t != u;
}

#define local_inc_not_zero(l) local_add_unless((l), 1, 0)

#define local_sub_and_test(a, l)	(local_sub_return((a), (l)) == 0)
#define local_dec_and_test(l)		(local_dec_return((l)) == 0)

/*
 * Atomically test *l and decrement if it is greater than 0.
 * The function returns the old value of *l minus 1.
 */
static __inline__ int local_dec_if_positive(local_t *l)
{
	int t;

	__asm__ __volatile__(
"1:	lwarx	%0,0,%1		# local_dec_if_positive\n\
	addic.	%0,%0,-1\n\
	blt-	2f\n"
	PPC405_ERR77(0,%1)
"	stwcx.	%0,0,%1\n\
	bne-	1b"
	"\n\
2:"	: "=&r" (t)
	: "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

#else /* __powerpc64__ */

static __inline__ long local_add_return(long a, local_t *l)
{
	long t;

	__asm__ __volatile__(
"1:	ldarx	%0,0,%2		# local_add_return\n\
	add	%0,%1,%0\n\
	stdcx.	%0,0,%2 \n\
	bne-	1b"
	: "=&r" (t)
	: "r" (a), "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

#define local_add_negative(a, l)	(local_add_return((a), (l)) < 0)

static __inline__ long local_sub_return(long a, local_t *l)
{
	long t;

	__asm__ __volatile__(
"1:	ldarx	%0,0,%2		# local_sub_return\n\
	subf	%0,%1,%0\n\
	stdcx.	%0,0,%2 \n\
	bne-	1b"
	: "=&r" (t)
	: "r" (a), "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

static __inline__ long local_inc_return(local_t *l)
{
	long t;

	__asm__ __volatile__(
"1:	ldarx	%0,0,%1		# local_inc_return\n\
	addic	%0,%0,1\n\
	stdcx.	%0,0,%1 \n\
	bne-	1b"
	: "=&r" (t)
	: "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

/*
 * local_inc_and_test - increment and test
 * @l: pointer of type local_t
 *
 * Atomically increments @l by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
#define local_inc_and_test(l) (local_inc_return(l) == 0)

static __inline__ long local_dec_return(local_t *l)
{
	long t;

	__asm__ __volatile__(
"1:	ldarx	%0,0,%1		# local_dec_return\n\
	addic	%0,%0,-1\n\
	stdcx.	%0,0,%1\n\
	bne-	1b"
	: "=&r" (t)
	: "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

#define local_sub_and_test(a, l)	(local_sub_return((a), (l)) == 0)
#define local_dec_and_test(l)	(local_dec_return((l)) == 0)

/*
 * Atomically test *l and decrement if it is greater than 0.
 * The function returns the old value of *l minus 1.
 */
static __inline__ long local_dec_if_positive(local_t *l)
{
	long t;

	__asm__ __volatile__(
"1:	ldarx	%0,0,%1		# local_dec_if_positive\n\
	addic.	%0,%0,-1\n\
	blt-	2f\n\
	stdcx.	%0,0,%1\n\
	bne-	1b"
	"\n\
2:"	: "=&r" (t)
	: "r" (&(l->a.counter))
	: "cc", "memory");

	return t;
}

#define local_cmpxchg(l, o, n) \
	((__typeof__((l)->a.counter))cmpxchg_local(&((l)->a.counter), (o), (n)))
#define local_xchg(l, new) (xchg_local(&((l)->a.counter), new))

/**
 * local_add_unless - add unless the number is a given value
 * @l: pointer of type local_t
 * @a: the amount to add to l...
 * @u: ...unless l is equal to u.
 *
 * Atomically adds @a to @l, so long as it was not @u.
 * Returns non-zero if @l was not @u, and zero otherwise.
 */
static __inline__ int local_add_unless(local_t *l, long a, long u)
{
	long t;

	__asm__ __volatile__ (
"1:	ldarx	%0,0,%1		# local_add_unless\n\
	cmpd	0,%0,%3 \n\
	beq-	2f \n\
	add	%0,%2,%0 \n"
	PPC405_ERR77(0,%2)
"	stdcx.	%0,0,%1 \n\
	bne-	1b \n"
"	subf	%0,%2,%0 \n\
2:"
	: "=&r" (t)
	: "r" (&(l->a.counter)), "r" (a), "r" (u)
	: "cc", "memory");

	return t != u;
}

#define local_inc_not_zero(l) local_add_unless((l), 1, 0)

#endif /* !__powerpc64__ */

/* Use these for per-cpu local_t variables: on some archs they are
 * much more efficient than these naive implementations.  Note they take
 * a variable, not an address.
 *
 * This could be done better if we moved the per cpu data directly
 * after GS.
 */

#define __local_inc(l)		((l)->a.counter++)
#define __local_dec(l)		((l)->a.counter++)
#define __local_add(i,l)	((l)->a.counter+=(i))
#define __local_sub(i,l)	((l)->a.counter-=(i))

/* Need to disable preemption for the cpu local counters otherwise we could
   still access a variable of a previous CPU in a non atomic way. */
#define cpu_local_wrap_v(l)	 	\
	({ local_t res__;		\
	   preempt_disable(); 		\
	   res__ = (l);			\
	   preempt_enable();		\
	   res__; })
#define cpu_local_wrap(l)		\
	({ preempt_disable();		\
	   l;				\
	   preempt_enable(); })		\

#define cpu_local_read(l)    cpu_local_wrap_v(local_read(&__get_cpu_var(l)))
#define cpu_local_set(l, i)  cpu_local_wrap(local_set(&__get_cpu_var(l), (i)))
#define cpu_local_inc(l)     cpu_local_wrap(local_inc(&__get_cpu_var(l)))
#define cpu_local_dec(l)     cpu_local_wrap(local_dec(&__get_cpu_var(l)))
#define cpu_local_add(i, l)  cpu_local_wrap(local_add((i), &__get_cpu_var(l)))
#define cpu_local_sub(i, l)  cpu_local_wrap(local_sub((i), &__get_cpu_var(l)))

#define __cpu_local_inc(l)	cpu_local_inc(l)
#define __cpu_local_dec(l)	cpu_local_dec(l)
#define __cpu_local_add(i, l)	cpu_local_add((i), (l))
#define __cpu_local_sub(i, l)	cpu_local_sub((i), (l))

#endif /* _ARCH_POWERPC_LOCAL_H */