// SPDX-License-Identifier: GPL-2.0
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#include <hyperenclave/log.h>

#include "feature.h"

/*
 * HHBox log feature: record hypervisor log when system is normal,
 * default enable.
 */
bool hhbox_log_enabled = true;

/*
 * HHBox crash feature: cope hypervisor panic and record hypervisor log when
 * hypervisor is abnormal, default disable. Besides, HHBox crash feature is
 * based on log feature, so if enable crash feature, log feature is enabled
 * default.
 */
bool hhbox_crash_enabled = false;

/*
 * Stats level to control which to record, default is level 0, record nothing.
 * Level 1 record EPC overcommit stats, level 2 record EPC overcommit stats and
 * related operation time stats.
 */
int stats_level = 0;

/*
 * shmem_pinned to control whether shared memory is pinned, default is false,
 * unpin shared memory. shmem_pinned = true means pin shared memory.
 */
bool shmem_pinned = false;

/*
 * EDMM feature: Enclave Dynamic Memory Management,
 * default enable.
 */
bool edmm_enabled = true;

/*
 * TPM feature: fake a TPM in a scenario without HW TPM and FTPM
 * default disable.
 */
bool fake_tpm = false;

/*
 * Memory test feature: Perform memory test before starting hypervisor,
 * default disable.
 */
bool memory_test_enabled = false;

// 初始化 feature (?) 
// 输入 mask 定义了该模块应当支持的 feature 
unsigned long long feature_init(unsigned long long mask)
{
	// 逆天写法，把 -1 当 unsigned long long 用
	// 无法确定它的计算路径是 int -> long long -> unsigned 
	// 还是 int -> unsigned -> unsigned long long 
	// ...... 
	if (mask == FEATURE_MASK_INVALID) {
		return FEATURE_MASK_DEFAULT;
	}

	// 这里更是没必要进行 if 判断
	hhbox_log_enabled = ( (mask & HHBOX_LOG_MASK) != 0 ); 
	// if (mask & HHBOX_LOG_MASK)
	// 	hhbox_log_enabled = true;
	// else
	// 	hhbox_log_enabled = false;

	hhbox_crash_enabled = ( ( mask & HHBOX_CRASH_MASK ) != 0 ); 
	if (hhbox_crash_enabled) {
		hhbox_log_enabled = true;
		// 怎么还修改呢... 
		mask |= HHBOX_LOG_MASK;
	}
	// if (mask & HHBOX_CRASH_MASK) {
	// 	hhbox_log_enabled = true;
	// 	hhbox_crash_enabled = true;
	// 	mask |= HHBOX_LOG_MASK;
	// } else
	// 非对称码风
	// 	hhbox_crash_enabled = false;

	// 从 mask 中提取出相关于加密算法的位
	if (get_reclaim_crypto_alg(mask) >= RECLAIM_CRYPTO_TYPES) {
		he_info("Invalid reclaim crypto algorithm configuration, use default HmacSW-then-EncHW");
		mask &= ~RECLAIM_CRYPTO_ALG_MASK;
	}


	stats_level = get_stats_level(mask);
	if (stats_level > 2) {
		stats_level = 0;
	}

	// 有点没明白，这些变量似乎都有初始值，通过在 .c 文件中进行提前指定，那这里完整地走两个分支的好处是什么？
	// 便于让类型显得更显然？
	shmem_pinned = ((mask & PIN_SHARED_MEMORY_MASK) != 0); 
	// if (mask & PIN_SHARED_MEMORY_MASK)
	// 	shmem_pinned = true;
	// else
	// 	shmem_pinned = false;

	if (mask & EDMM_OFF_MASK)
		edmm_enabled = false;

	if (mask & FAKE_TPM_MASK)
		fake_tpm = true;

	if (mask & MEM_TEST_MASK)
		memory_test_enabled = true;

	return mask;
}
