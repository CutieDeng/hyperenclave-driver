/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#pragma once 

/// @brief 内存区域映射描述，记录了内存的物理地址、虚拟地址、长度和特殊标识
struct memory_region {
	__u64 phys_start;
	__u64 virt_start;
	__u64 size;
	__u64 flags;
} __packed;

/// @brief 类 span. 
struct memory_range {
	__u64 start;
	__u64 size;
};

