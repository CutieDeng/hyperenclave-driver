/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#pragma once 

#ifdef CONFIG_X86
unsigned long get_sme_mask(void);
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
unsigned long get_sme_mask(void)
{
    he_info("AArch64 CPU does not enable SME\n");
    return 0; 
}
#else 
#error "unimpl get sme mask" 
#endif
