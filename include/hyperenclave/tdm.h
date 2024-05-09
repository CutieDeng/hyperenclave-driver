/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#pragma once 

// 包含了 ShangMi3 crytograhpic hash function 
#include <crypto/sm3.h>
// Linux 内核标准字符串操作
#include <linux/string.h>

// 
#include "hyperenclave/memory.h" 

// 类型 tdm: 
struct tdm;
extern struct tdm tdm;

// 可能是 HyperVisor 进程信息的初始化，疑似单例模式
int proc_hypervisorinfo_init(void);
// 可能是 HyperVisor 进程信息的移除操作？且单例模式（那听起来没必要
void proc_hypervisorinfo_remove(void);

// measure image 为什么 size 用了 `unsigned int`? 感觉有点问题呀...
int measure_image(unsigned char *start_addr, unsigned int size,
		  unsigned char *digest);
// ??? 
void tdm_init(void);

// tdm 操作？一种虚函数表
struct tdm_ops {
	int (*proc_init)(void);
	void (*proc_remove)(void);
	void (*measure)(void);
	bool (*is_tdm_info_init)(void);
	void (*set_tdm_info)(struct memory_region *region);
	void (*clear_tdm_info)(void);
	unsigned long long (*get_tdm_phys_addr)(void);
	unsigned long long (*get_tdm_virt_addr)(void);
	unsigned long long (*get_tdm_size)(void);
};

/// @brief Trusted Domain Manager (TDM) 
struct tdm {
	/// @brief 相关的 hypervisor 内存区域？
	struct memory_region hv_mem;
	/// @brief 记录 hv_mem 的内存摘要
	unsigned char hv_digest[SM3_DIGEST_SIZE];
	/// 虚函数表
	struct tdm_ops const *ops;
};
