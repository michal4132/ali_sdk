#include "mipsregs.h"
#include "regs.h"

#define TLB_TABLE_NUM 64
#define PAGE16K_MASK (0x03 << 13)

#define		ST0_IE		(1 << 0)
#define		ST0_EXL		(1 << 1)
#define		ST0_ERL		(1 << 2)
#define		ST0_KSU		(3 << 3)
#define		ST0_UM		(1 << 4)
#define		ST0_IM0		(1 << 8)
#define		ST0_IM1		(1 << 9)
#define		ST0_IM2		(1 <<10)
#define		ST0_IM3		(1 <<11)
#define		ST0_IM4		(1 <<12)
#define		ST0_IM5		(1 <<13)
#define		ST0_IM6		(1 <<14)
#define		ST0_IM7		(1 <<15)
#define		ST0_NMI		(1 <<19)
#define		ST0_SR		(1 <<20)
#define		ST0_TS		(1 <<21)
#define		ST0_BEV		(1 <<22)
#define		ST0_FR		(1 <<26)
#define		ST0_RP		(1 <<27)
#define		ST0_CU0		(1 <<28)
#define		ST0_CU1		(1 <<29)

// FIXME: this is cpu/subarch specific
#define	ST0_CLRMSK	(ST0_KSU|ST0_BEV|ST0_ERL|ST0_EXL|ST0_IE)
#define	ST0_SETMSK	0
#define	c0_status	__COPRO_REG($12, 0)

#ifdef	__ASSEMBLY__
#define	__COPRO_REG(R,S)	R, S
#else
#define	__COPRO_REG(R,S)	#R ", " #S
#endif

.globl _start
.ent _start
.text
_start:

	# # init the status register
	# mfc0	t0, Status, 0
	# or		t0, t0, ST0_CLRMSK|ST0_SETMSK
	# xor		t0, t0, ST0_CLRMSK
	# mtc0	t0, Status, 0
	# ehb


	# init the status register
	mfc0	t0, Config
    li		t1, -0x8
	and		t0, t0, t1
	ori		t0, t0, 0x3
	mtc0	t0, Config
	mfc0	t0, Config7
	ori		t0, t0, 0x30
	mtc0	t0, Config7

	mtc0	zero, Cause, 0
	ehb

	mtc0    zero, Count
	mtc0    a0, Compare
	ehb

	mtc0	zero, TagLo
	mtc0	zero, TagHi

	# clear cp0 context
	mtc0	$0, Context, 0
	ehb

	# enable co1
	mfc0	t0, Status, 0
	li		t1, ST0_CU1
	or		t0, t1
	mtc0	t0, Status, 0
	ehb


	# CP1_STATUS
	ctc1	zero, $31
	ctc1	zero, $28

	li		t0, -1

	mtc1	t0, $f0
	mtc1	t0, $f1
	mtc1	t0, $f2
	mtc1	t0, $f3
	mtc1	t0, $f4
	mtc1	t0, $f5
	mtc1	t0, $f6
	mtc1	t0, $f7
	mtc1	t0, $f8
	mtc1	t0, $f9
	mtc1	t0, $f10
	mtc1	t0, $f11
	mtc1	t0, $f12
	mtc1	t0, $f13
	mtc1	t0, $f14
	mtc1	t0, $f15
	mtc1	t0, $f16
	mtc1	t0, $f17
	mtc1	t0, $f18
	mtc1	t0, $f19
	mtc1	t0, $f20
	mtc1	t0, $f21
	mtc1	t0, $f22
	mtc1	t0, $f23
	mtc1	t0, $f24
	mtc1	t0, $f25
	mtc1	t0, $f26
	mtc1	t0, $f27
	mtc1	t0, $f28
	mtc1	t0, $f29
	mtc1	t0, $f30
	mtc1	t0, $f31

	# tlb_init
	li		a0, TLB_TABLE_NUM
	li		a1, 0
	li		v0, PAGE16K_MASK

	mtc0	$0, CP0_ENTRYHI
	mtc0	$0, CP0_ENTRYLO0
	mtc0	$0, CP0_ENTRYLO1
	mtc0	v0, CP0_PAGEMASK
1:
	mtc0	a1, CP0_INDEX
	addiu	a1, a1, 1
	bne		a1, a0, 1b
	tlbwi
	nop
	
	# Set up a stack 
	la		sp, _estack

	#  Jump to C
	la		t0, entrypoint
	jr		t0
	nop

.end _start
