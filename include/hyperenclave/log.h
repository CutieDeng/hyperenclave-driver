/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */
#pragma once 

#include <linux/printk.h>

// cutie: definition of pr_fmt? 

#undef pr_fmt
#define pr_fmt(fmt) "HE: %s: %d. " fmt, __func__, __LINE__

#define he_debug(fmt...) pr_debug(fmt)
#define he_info(fmt...) pr_info(fmt)
#define he_warn(fmt...) pr_warn("HE_WARN. " fmt)
#define he_err(fmt...) pr_err("HE_ERROR. " fmt)
