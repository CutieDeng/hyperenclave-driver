/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#pragma once 

// ARM 上 SME 支持为空
unsigned long get_sme_mask(void);

#ifdef CONFIG_X86
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#else 
#error "unimpl get sme mask" 
#endif
