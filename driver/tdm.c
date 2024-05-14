// SPDX-License-Identifier: GPL-2.0
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#include <hyperenclave/tdm.h>
#include <hyperenclave/vendor.h>

extern const struct tdm_ops hygon_tdm_ops;

struct tdm tdm; 

static int generic_proc_init(void)
{
	return 0;
}

static void generic_proc_remove(void)
{
}

static void generic_tdm_measure(void)
{
}

static bool generic_is_tdm_info_init(void)
{
	return false;
}

static void generic_set_tdm_info(struct memory_region *region)
{
}

static void generic_clear_tdm_info(void)
{
}

static unsigned long long generic_get_tdm_phys_addr(void)
{
	return 0;
}

static unsigned long long generic_get_tdm_virt_addr(void)
{
	return 0;
}

static unsigned long long generic_get_tdm_size(void)
{
	return 0;
}

/// @brief 默认虚表就跟全 NULL 有啥区别？为什么要搞这么复杂... 
/// 直接把 ops 设成 NULL 得了还不如..
/// 这样还可以确定是否有 trust domain manager 实装
const struct tdm_ops generic_tdm_ops = {
	.proc_init = generic_proc_init,
	.proc_remove = generic_proc_remove,
	.measure = generic_tdm_measure,
	.is_tdm_info_init = generic_is_tdm_info_init,
	.set_tdm_info = generic_set_tdm_info,
	.clear_tdm_info = generic_clear_tdm_info,
	.get_tdm_phys_addr = generic_get_tdm_phys_addr,
	.get_tdm_virt_addr = generic_get_tdm_virt_addr,
	.get_tdm_size = generic_get_tdm_size,
};

/// @brief 根据当前 cpu 型号，即 vendor, 设置相应的 tdm ops. 
/// 事实上，只有 cpu vendor 为 HE_X86_VENDOR_HYGON 时，才有对 tdm ops 的特化代码：hygon_tdm_osp; 
/// 否则，退化到通用调用 generic_tdm_ops. 
/// @param   
void tdm_init(void)
{
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64) 
	tdm.ops = &generic_tdm_ops;
#elif defined(CONFIG_X86) 
	if (vendor == HE_X86_VENDOR_HYGON) {
		tdm.ops = &hygon_tdm_ops;
	} else {
		tdm.ops = &generic_tdm_ops;
	}
#endif 
}
