/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */
#pragma once 

//! 我想了一下，其实这个模块本身就没什么意义，完全移除也是可以的.. 
//! 毕竟它只是讲了 tdm_hv 的一些操作，并且也没有什么事实上的封装，搞得挺搞笑，只能说。

#include <crypto/sm3.h>
#include <linux/string.h>

#include <hyperenclave/memory.h>

/// 该不会这里就是变量的定义吧？
/// 查了一下文件，没有 tdm_hv 的结构定义. 
/// 疑点 tag:todo
extern struct memory_region tdm_hv;

// 无意义 extern, 给你扬了哈
// extern unsigned char tdm_hv_digest[SM3_DIGEST_SIZE];

/// @brief 有点疑或，这里的 tdm 是指 tdm handle 所操作的那个结构是否被初始化
/// 可是如果按这个逻辑，为什么不考虑设置成拿到 tdm handle 之后它必然是有效的呢？
/// 如果是想询问当前 tdm handle 是不是一个有效、可用的 handle, 这个检查恐怕是不可能的
/// 如果想允许这个检查，无疑要实现一个 tdm handle 的 invalid 状态设置——这听起来太蠢了，设计错误 
/// 尽管确实可以利用 tdm handle 本身的一些理论上无效的字段值来完成这件事，正如这里的 `size` 字段一样
/// 但面向对象的封装看起来并不真的成功
/// @param 
/// @return 
static inline bool is_tdm_info_init(void)
{
	return !!tdm_hv.size;
}

/// @brief 直接设置 tdm 上的信息
/// @param region 
static inline void set_tdm_info(struct memory_region *region)
{
	// 这不就完事了？`memcpy` 是什么逆天操作，这才几个字节？
	tdm_hv = *region; 
	// memcpy(&tdm_hv, region, sizeof(tdm_hv));
}

/// @brief 同理，清理各字段（其实只需要清理 size 字段；其他字段是不需要清理的
/// @param  
static inline void clear_tdm_info(void)
{
	tdm_hv.size = 0; 
	// memset(&tdm_hv, 0, sizeof(tdm_hv));
}

static inline unsigned long long get_tdm_phys_addr(void)
{
	return tdm_hv.phys_start;
}

static inline unsigned long long get_tdm_virt_addr(void)
{
	return tdm_hv.virt_start;
}

static inline unsigned long long get_tdm_size(void)
{
	return tdm_hv.size;
}
