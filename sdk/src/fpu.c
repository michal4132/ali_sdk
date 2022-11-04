#include "fpu.h"
#include "mipsregs.h"
#include "regs.h"
#include <signal.h>

int __enable_fpu(enum fpu_mode mode) {
    int fr;

    switch (mode) {
    case FPU_AS_IS:
        /* just enable the FPU in its current mode */
        c0_setbits(CP0_STATUS, ST0_CU1);
        enable_fpu_hazard();
        return 0;

    case FPU_HYBRID:
        // if (!cpu_has_fre)
        // return SIGFPE;

        /* set FRE */
        c0_setbits(CP0_CONFIG5, CFG5_FRE);
        goto fr_common;

    case FPU_64BIT:
// #if !(defined(CONFIG_CPU_MIPSR2) || defined(CONFIG_CPU_MIPSR5) ||              
//       defined(CONFIG_CPU_MIPSR6) || defined(CONFIG_64BIT))
//         /* we only have a 32-bit FPU */
//         return SIGFPE;
// #endif
        /* fallthrough */
    case FPU_32BIT:
        // TODO: cpu_has_fre
        // if (cpu_has_fre) {
        /* clear FRE */
        // c0_clrbits(CP0_CONFIG5, CFG5_FRE);
        // }
    fr_common:
        /* set CU1 & change FR appropriately */
        fr = (int)mode & FPU_FR_MASK;
        c0_chgbits(CP0_STATUS, ST0_CU1 | ST0_FR, ST0_CU1 | (fr ? ST0_FR : 0));
        enable_fpu_hazard();

        /* check FR has the desired value */
        if (!!(c0_getval(CP0_STATUS) & ST0_FR) == !!fr)
            return 0;

        /* unsupported FR value */
        // __disable_fpu();
        return SIGFPE;
    }

    return SIGFPE;
}

/*
 * Get the FPU Implementation/Revision.
 */
unsigned long cpu_get_fpu_id(void) {
    unsigned long tmp, fpu_id;

    tmp = c0_getval(CP0_STATUS);
    __enable_fpu(FPU_AS_IS);
    fpu_id = read_32bit_cp1_register(CP1_REVISION);
    c0_setval(CP0_STATUS, tmp);
    return fpu_id;
}

int __cpu_has_fpu(void) {
    return (cpu_get_fpu_id() & FPIR_IMP_MASK) != FPIR_IMP_NONE;
}