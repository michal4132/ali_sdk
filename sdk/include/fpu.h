#ifndef __FPU_H__
#define __FPU_H__

/*
 * Coprocessor 1 (FPU) register names
 */
#define CP1_REVISION	$0
#define CP1_UFR		$1
#define CP1_UNFR	$4
#define CP1_FCCR	$25
#define CP1_FEXR	$26
#define CP1_FENR	$28
#define CP1_STATUS	$31

#define FPIR_IMP_MASK 0xff00

#define FPIR_IMP_NONE 0x0000


#define enable_fpu_hazard()		    			\
do {								        	\
	__asm__ __volatile__("ehb");				\
} while (0)

enum fpu_mode {
    FPU_32BIT = 0, /* FR = 0 */
    FPU_64BIT,     /* FR = 1, FRE = 0 */
    FPU_AS_IS,
    FPU_HYBRID, /* FR = 1, FRE = 1 */

#define FPU_FR_MASK 0x1
};

#ifdef GAS_HAS_SET_HARDFLOAT
#define read_32bit_cp1_register(source)					\
	_read_32bit_cp1_register(source, .set hardfloat)
#define write_32bit_cp1_register(dest, val)				\
	_write_32bit_cp1_register(dest, val, .set hardfloat)
#else
#define read_32bit_cp1_register(source)					\
	_read_32bit_cp1_register(source, )
#define write_32bit_cp1_register(dest, val)				\
	_write_32bit_cp1_register(dest, val, )
#endif

/*
 * Macros to access the floating point coprocessor control registers
 */
#define _read_32bit_cp1_register(source, gas_hardfloat)			\
({									\
	unsigned int __res;						\
									\
	__asm__ __volatile__(						\
	"	.set	push					\n"	\
	"	.set	reorder					\n"	\
	"	# gas fails to assemble cfc1 for some archs,	\n"	\
	"	# like Octeon.					\n"	\
	"	.set	mips1					\n"	\
	"	"STR(gas_hardfloat)"				\n"	\
	"	cfc1	%0,"STR(source)"			\n"	\
	"	.set	pop					\n"	\
	: "=r" (__res));						\
	__res;								\
})

#define _write_32bit_cp1_register(dest, val, gas_hardfloat)		\
do {									\
	__asm__ __volatile__(						\
	"	.set	push					\n"	\
	"	.set	reorder					\n"	\
	"	"STR(gas_hardfloat)"				\n"	\
	"	ctc1	%0,"STR(dest)"				\n"	\
	"	.set	pop					\n"	\
	: : "r" (val));							\
} while (0)

int __enable_fpu(enum fpu_mode mode);
unsigned long cpu_get_fpu_id(void);
int __cpu_has_fpu(void);
#endif // __FPU_H__