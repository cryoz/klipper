// Reboot into stm32 ROM dfu bootloader
//
// Copyright (C) 2019-2022  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "internal.h" // NVIC_SystemReset
#include "board/irq.h" // irq_disable

// Many stm32 chips have a USB capable "DFU bootloader" in their ROM.
// In order to invoke that bootloader it is necessary to reset the
// chip and jump to a chip specific hardware address.
//
// To reset the chip, the dfu_reboot() code sets a flag in memory (at
// an arbitrary position that is unlikely to be overwritten during a
// chip reset), and resets the chip.  If dfu_reboot_check() sees that
// flag on the next boot it will perform a code jump to the ROM
// address.

// Location of ram address to set internal flag
#define USB_BOOT_FLAG_ADDR (CONFIG_RAM_START + CONFIG_RAM_SIZE - 512)

// Signature to set in memory to flag that a dfu reboot is requested
#define USB_BOOT_FLAG 0x4254 // "BT"

// Flag that bootloader is desired and reboot
void
dfu_reboot(void)
{
    if (!CONFIG_GD32_DFU_ROM_ADDRESS)
        return;
    irq_disable();
#if CONFIG_MACH_GD32F30X
    BKP_DATA1 = USB_BOOT_FLAG;
#elif CONFIG_MACH_GD32E23X
    RTC_BKP1 = USB_BOOT_FLAG;
#else
    uint64_t *bflag = (void*)USB_BOOT_FLAG_ADDR;
    *bflag = USB_BOOT_FLAG;
#endif
    NVIC_SystemReset();
}

// Check if rebooting into system DFU Bootloader
void
dfu_reboot_check(void)
{
    if (!CONFIG_GD32_DFU_ROM_ADDRESS)
        return;
#if CONFIG_MACH_GD32E23X
    if (RTC_BKP1 != USB_BOOT_FLAG)
        return;
    RTC_BKP1 = 0;
#elif CONFIG_MACH_GD32F30X
    if (BKP_DATA1 != USB_BOOT_FLAG)
        return;
    BKP_DATA1 = 0;
#else
    if(*(uint64_t*)USB_BOOT_FLAG_ADDR != USB_BOOT_FLAG)
	return;
    *(uint64_t*)USB_BOOT_FLAG_ADDR = 0;
#endif

    uint32_t *sysbase = (uint32_t*)CONFIG_GD32_DFU_ROM_ADDRESS;
    asm volatile("mov sp, %0\n bx %1"
                 : : "r"(sysbase[0]), "r"(sysbase[1]));
}
