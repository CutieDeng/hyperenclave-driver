// SPDX-License-Identifier: GPL-2.0
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

// #include <asm/msr.h>
#include <linux/processor.h>

#include <hyperenclave/log.h>
#include <hyperenclave/vendor.h>

/*
 * SME reference:
 * https://www.kernel.org/doc/html/latest/x86/amd-memory-encryption.html
 */
#define HE_MSR_K8_SYSCFG 0xc0010010
#define HE_MSR_K8_SYSCFG_MEM_ENCRYPT (1 << 23)
#define HE_CPUID_GUEST_MODE (1 << 31)

unsigned long get_sme_mask(void)
{
	he_info("AArch64 cpu do not support sme ext. \n"); 
	return 0; 
} 