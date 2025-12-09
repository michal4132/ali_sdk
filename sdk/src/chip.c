
#include "chip.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#ifdef  ALI_ARM_STB
#define ALI_SOC_BASE			              0x18000000	// 3921 arm
#else
#define ALI_SOC_BASE			              0xb8000000	// mips
#endif

static UINT32 reg_88, reg_ac, reg_2e000, reg_70;

UINT8 readb(UINT32 addr)
{
	return *(const volatile UINT8 *) addr;
}

UINT16 readw(UINT32 addr)
{
	return *(const volatile UINT16 *) addr;
}

UINT32 readl(UINT32 addr)
{
	return *(const volatile UINT32 *) addr;
}

void writeb(UINT8 b, UINT32 addr)
{
	*(volatile UINT8 *) addr = b;
}

void writew(UINT16 b, UINT32 addr)
{
	*(volatile UINT16 *) addr = b;
}

void writel(UINT32 b, UINT32 addr)
{
	*(volatile UINT32 *) addr = b;
}

UINT32 sys_ic_get_chip_id_raw()   
{  
    unsigned int id = readl(ALI_SOC_BASE)  >> 16;
	return id;
}
UINT32 sys_ic_get_rev_id()   
{
    unsigned int id = readl(ALI_SOC_BASE);
   // if((id&0xff) == 0)
   /// 	return IC_REV_0;
    //else
    	return id;
}

void stop_watchdog()
{
	*(volatile unsigned char *)(ALI_SOC_BASE + 0x18504) = 0;	
}
static void wdt_reboot_from_nand(void)
{     
     *(volatile unsigned int *)(ALI_SOC_BASE + 0x74) |= 1<<18;  
     *(volatile unsigned int *)(0x18000074) |= 1<<30;      
}

static void wdt_reboot_from_nor(void)
{   
   *(volatile unsigned int *)(ALI_SOC_BASE+0x74) &= 0xFFFBFFFF;       
   *(volatile unsigned int *)(ALI_SOC_BASE+0x74) |= 1<<30;  
}

void hw_watchdog_reboot()
{
#if 0
#ifdef NAND_BOOT
    sys_ic_change_boot_type(BOOT_TYPE_NAND);
#else
    sys_ic_change_boot_type(BOOT_TYPE_NOR);
#endif
#endif
   *(volatile unsigned int *)(ALI_SOC_BASE+0x18500) = 0xfffff000;    
   *(volatile unsigned int *)(ALI_SOC_BASE+0x18504) = 0x67;  
   do{}while(1);
}

/*
void ali_see_stop()
{
    *(volatile unsigned int *)(ALI_SOC_BASE+0x40038) =0 ; // disable see interrupt
    *(volatile unsigned int *)(ALI_SOC_BASE+0x4003C) =0 ; // disable see interrupt
    osal_delay(10);
    *(volatile unsigned int *)(ALI_SOC_BASE+0x220) &=~(0x1 <<1) ;// dis see 
    *(volatile UINT32*)(ALI_SOC_BASE+0x20C) |= 1 ; // cold boot up flag 
}
*/



