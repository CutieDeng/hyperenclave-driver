/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#pragma once 

#ifdef CONFIG_X86
unsigned long get_sme_mask(void);
#else
unsigned long get_sme_mask(void)
{
}
#endif
