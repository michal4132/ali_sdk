
/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

#define c0_getval(REG_SEL)                                                     \
    ({                                                                         \
        unsigned long __val;                                                   \
        asm volatile("mfc0	%0, " REG_SEL : "=r"(__val));                      \
        __val;                                                                 \
    })

#define c0_setval(REG_SEL, VAL)                                                \
    ({                                                                         \
        unsigned long __val = VAL;                                             \
        asm volatile("mtc0	%0, " REG_SEL "	\n\t"                              \
                     "ehb"                                                     \
                     :                                                         \
                     : "r"(__val));                                            \
    })

#define c0_setbits(REG_SEL, SET)                                               \
    ({                                                                         \
        unsigned long __old, __new;                                            \
        __old = c0_getval(REG_SEL);                                            \
        __new = __old | (SET);                                                 \
        c0_setval(REG_SEL, __new);                                             \
        __old;                                                                 \
    })

#define c0_clrbits(REG_SEL, CLR)                                               \
    ({                                                                         \
        unsigned long __old, __new;                                            \
        __old = c0_getval(REG_SEL);                                            \
        __new = __old & ~(CLR);                                                \
        c0_setval(REG_SEL, __new);                                             \
        __old;                                                                 \
    })

#define c0_chgbits(REG_SEL, MSK, VAL)                                          \
    ({                                                                         \
        unsigned long __old, __new;                                            \
        __old = c0_getval(REG_SEL);                                            \
        __new = (__old & ~(MSK)) | ((VAL) & (MSK));                            \
        c0_setval(REG_SEL, __new);                                             \
        __old;                                                                 \
    })

#define exec(addr) ({ __asm__ __volatile__("jr	%0\n" : : "r"(addr)); })

#define read_32bit_cause_register()                                            \
    ({                                                                         \
        unsigned int __res;                                                    \
                                                                               \
        __asm__ __volatile__("mfc0    %0, $13 \n" : "=r"(__res));              \
        __res;                                                                 \
    })

/*
 * Coprocessor 0 register names
 */
#define CP0_INDEX $0
#define CP0_RANDOM $1
#define CP0_ENTRYLO0 $2
#define CP0_ENTRYLO1 $3
#define CP0_CONF $3
#define CP0_GLOBALNUMBER $3, 1
#define CP0_CONTEXT $4
#define CP0_PAGEMASK $5
#define CP0_PAGEGRAIN $5, 1
#define CP0_SEGCTL0 $5, 2
#define CP0_SEGCTL1 $5, 3
#define CP0_SEGCTL2 $5, 4
#define CP0_WIRED $6
#define CP0_INFO $7
#define CP0_HWRENA $7
#define CP0_BADVADDR $8
#define CP0_BADINSTR $8, 1
#define CP0_COUNT $9
#define CP0_ENTRYHI $10
#define CP0_GUESTCTL1 $10, 4
#define CP0_GUESTCTL2 $10, 5
#define CP0_GUESTCTL3 $10, 6
#define CP0_COMPARE $11
#define CP0_GUESTCTL0EXT $11, 4
#define CP0_STATUS "$12, 0"
#define CP0_GUESTCTL0 $12, 6
#define CP0_GTOFFSET $12, 7
#define CP0_CAUSE $13
#define CP0_EPC $14
#define CP0_PRID $15
#define CP0_EBASE $15, 1
#define CP0_CMGCRBASE $15, 3
#define CP0_CONFIG $16
#define CP0_CONFIG3 $16, 3
#define CP0_CONFIG5 "$16, 5"
#define CP0_CONFIG6 $16, 6
#define CP0_LLADDR $17
#define CP0_WATCHLO $18
#define CP0_WATCHHI $19
#define CP0_XCONTEXT $20
#define CP0_FRAMEMASK $21
#define CP0_DIAGNOSTIC $22
#define CP0_DEBUG $23
#define CP0_DEPC $24
#define CP0_PERFORMANCE $25
#define CP0_ECC $26
#define CP0_CACHEERR $27
#define CP0_TAGLO $28
#define CP0_TAGHI $29
#define CP0_ERROREPC $30
#define CP0_DESAVE $31

#define zero $0
#define at $1 /* assembly temporary */
#define v0 $2 /* function return & expr eval */
#define v1 $3
#define a0 $4 /* function arguments */
#define a1 $5
#define a2 $6
#define a3 $7
#define t0 $8  /* temporary values */
#define t1 $9
#define t2 $10
#define t3 $11
#define t4 $12
#define t5 $13
#define t6 $14
#define t7 $15
#define s0 $16 /* callee-preserved temporary values */
#define s1 $17
#define s2 $18
#define s3 $19
#define s4 $20
#define s5 $21
#define s6 $22
#define s7 $23
#define t8 $24 /* more temporary values */
#define t9 $25
#define k0 $26 /* kernel only */
#define k1 $27
#define gp $28 /* global pointer */
#define sp $29 /* stack pointer */
#define fp $30 /* frame pointer */
#define ra $31 /* return address */

