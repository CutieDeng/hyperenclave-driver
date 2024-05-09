/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#pragma once 

/// 前版本的 cpu-vendor 只支持 x86 的定义，
/// 为了保证代码兼容性，进行额外的 ARM 选项拓展
enum cpu_vendor {
	HE_VENDOR_UNKNOWN,
	HE_X86_VENDOR_INTEL,
	HE_X86_VENDOR_AMD,
	HE_X86_VENDOR_HYGON,
	HE_X86_VENDOR_CENTAURHAULS,
	HE_VENDOR_MAX, 

	// 以下均为 ARM 支持
	HE_ARM_VENDOR_ARM, 
	HE_ARM_VENDOR_QUALCOMM,
    HE_ARM_VENDOR_SAMSUNG,
    HE_ARM_VENDOR_NVIDIA,
	// 为我保留
	RESERVED 
};

/// 全局 vendor, 描述当前 cpu 的型号
/// 在 tdm.c 中根据 cpu vendor 会设置全局不同的 tdm.ops 内容
/// 该值设置应晚于 tdm_init 调用
extern enum cpu_vendor vendor;

#ifdef CONFIG_X86

void cpu_vendor_detect(void);

#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64) 

// 虽然和 CONFIG_X86 没什么区别，但还是把分支设出来，以便未来更新
void cpu_vendor_detect(void);

#else 

static inline void cpu_vendor_detect(void) {} 

#endif
