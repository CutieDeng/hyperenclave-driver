// SPDX-License-Identifier: GPL-2.0
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#include <hyperenclave/log.h>
#include <hyperenclave/sme.h>

#include <linux/dma-mapping.h>
#include <linux/device.h> 
#include <linux/module.h> 

#include "main.h"
#include "init_mem.h"
#include "ioremap.h"

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64) 
static void wbinvd_on_all_cpus_impl(void *arg) { 
	// 执行数据缓存清除和无效化操作
    // `dsb` 用于数据同步障碍，确保缓存操作在继续之前完成
    // `isb` 用于指令同步障碍，确保所有先前指令完成
    // `dmb` 用于数据内存障碍，确保内存访问顺序一致
	asm volatile ("dsb sy":::"memory");
    asm volatile ("isb":::"memory");
    asm volatile ("dmb sy":::"memory");
	// ??? 
	if (arg) {
		asm volatile ("dc cisw, %0" :: "r" (arg) : "memory"); 
	}
}
void wbinvd_on_all_cpus(void ); 
void wbinvd_on_all_cpus() { 
	on_each_cpu(wbinvd_on_all_cpus_impl, NULL, 1); 
}
// help, 这个 api 什么意思？
#define __cpuc_flush_dcache_area() do { \
	on_each_cpu(wbinvd_on_all_cpus_impl, ((void * )addr), 1); \
} while (0); 

#endif 

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64) 
void flush_cache_area(void *addr, size_t size, struct device *dev)
{
	dma_sync_single_for_cpu(dev, virt_to_phys(addr), size, DMA_TO_DEVICE); 
	dma_sync_single_for_device(dev, virt_to_phys(addr), size, DMA_FROM_DEVICE); 
} 
#endif 

static unsigned long gen_magic_num(unsigned long addr)
{
	__cpuc_flush_dcache_area();	
	return addr;
}

bool mem_test(void)
{
	unsigned long sme_mask;
	unsigned long page_attr, magic_num;
	unsigned long phys;
	unsigned long mem_base_addr, *mem_ptr;
	unsigned long remain_size, batch_size, size;
	unsigned long region_idx, ptr_pos;

	// 疑似竞态访问，检查当前 hyper enclave 是否异步关闭
	// 但没有进行任何相关的线程安全控制
	// 简要地说，也许可以考虑加个 _Atomic ? 
	if (hyper_enclave_enabled) {
		he_err("Cannot perform memory test with hyper enclave enabled\n");
		return false;
	}

	// 
	/* Perform memory test in 8GB granularity */
	batch_size = 8L * 1024 * 1024 * 1024;
	page_attr = pgprot_val(PAGE_KERNEL);
	sme_mask = get_sme_mask();

	for (region_idx = 0; region_idx < nr_rsrv_mem; region_idx++) {
		phys = rsrv_mem_ranges[region_idx].start;
		remain_size = rsrv_mem_ranges[region_idx].size;

		while (remain_size > 0) {
			size = remain_size > batch_size ? batch_size : remain_size;
			mem_base_addr = (unsigned long)he_ioremap(
				phys, (unsigned long)NULL, size, sme_mask);

			he_info("Memory[0x%lx - 0x%lx] test begin\n", phys,
				phys + size);

			/* Set the memory */
			for (ptr_pos = 0; ptr_pos < size;
			     ptr_pos += sizeof(unsigned long)) {
				mem_ptr = (unsigned long *)(mem_base_addr +
							    ptr_pos);
				*mem_ptr = gen_magic_num(phys + ptr_pos);
			}
			wbinvd_on_all_cpus();

			/* Check whether the contents fetched from the memory equal to that written recently */
			for (ptr_pos = 0; ptr_pos < size;
			     ptr_pos += sizeof(unsigned long)) {
				mem_ptr = (unsigned long *)(mem_base_addr +
							    ptr_pos);
				magic_num = gen_magic_num(phys + ptr_pos);
				if (magic_num != *mem_ptr) {
					he_err("Memory test fails, phys addr[0x%lx]: 0x%lx != 0x%lx\n",
					       phys + ptr_pos, *mem_ptr,
					       magic_num);

					vunmap((void *)mem_base_addr);
					return false;
				}
			}

			memset((void *)mem_base_addr, 0, size);
			vunmap((void *)mem_base_addr);
			he_info("Memory[0x%lx - 0x%lx] test pass\n", phys,
				phys + size);

			remain_size -= size;
			phys += size;

			/*
			 * We may loop for a long time for testing the memory.
			 * Allow us to schedule out to avoid softlocking if preempt
			 * is disabled.
			 */
			cond_resched();
		}
	}

	return true;
}
