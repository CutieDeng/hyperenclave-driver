// SPDX-License-Identifier: GPL-2.0
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#include <linux/processor.h>

#include <hyperenclave/log.h>
#include <hyperenclave/vendor.h>

static const char *cpu_vendor_table[HE_VENDOR_MAX] = {
	"Unknown",
	"GenuineIntel",
	"AuthenticAMD",
	"HygonGenuine",
	"CentaurHauls",
};

enum cpu_vendor vendor;

#ifdef CONFIG_X86
void cpu_vendor_detect(void)
{
	int i;
	int cpuid_level;
	char x86_vendor_id[16] = "";
	/* Get vendor name */
	cpuid(0x00000000, (unsigned int *)&cpuid_level,
	      (unsigned int *)&x86_vendor_id[0],
	      (unsigned int *)&x86_vendor_id[8],
	      (unsigned int *)&x86_vendor_id[4]);

	for (i = 0; i < HE_VENDOR_MAX; i++) {
		if (strstr(x86_vendor_id, cpu_vendor_table[i]))
			vendor = i;
	}
	he_info("Vendor ID: %s\n", cpu_vendor_table[vendor]);
}
#endif

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)

/// @brief 获取 ARM 架构上的 cpu 型号
/// @param 
void cpu_vendor_detect(void)
{
    unsigned int midr_el1;
    unsigned int implementer_id, part_num;

	(void ) cpu_vendor_table; 

    /* Read MIDR_EL1 register to get CPU identification */
    asm("mrs %0, MIDR_EL1" : "=r" (midr_el1));

    /* Extract implementer and part number */
    implementer_id = (midr_el1 >> 24) & 0xFF;  // Implementer is in the upper byte
    part_num = (midr_el1 >> 4) & 0xFFF;       // Part number is a 12-bit value

    /* Match the implementer ID to known ARM vendors */
    switch (implementer_id) {
    case 0x41:  // 'A' for ARM
        switch (part_num) {
        // You can add more specific ARM cores here
        default:
            vendor = HE_ARM_VENDOR_ARM;
            break;
        }
        break;
    case 0x51:  // 'Q' for Qualcomm
        vendor = HE_ARM_VENDOR_QUALCOMM;
        break;
    case 0x53:  // 'S' for Samsung
        vendor = HE_ARM_VENDOR_SAMSUNG;
        break;
    case 0x4E:  // 'N' for NVIDIA
        vendor = HE_ARM_VENDOR_NVIDIA;
        break;
    default:
        vendor = HE_VENDOR_UNKNOWN;
        break;
    }

    he_info("Vendor ID: 0x%02x, Part Number: 0x%03x\n", implementer_id, part_num);
}

#endif
