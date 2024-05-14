// SPDX-License-Identifier: GPL-2.0
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

/// Advanced Configuration and Power Interface 
#include <linux/acpi.h>
#include <linux/sizes.h>

#include <hyperenclave/log.h>

#include "dmar.h"

// __init, 提示代码可以提前卸载 (?) 
static int __init check_dmar_checksum(struct acpi_table_header *table)
{
	int i;
	// C 语言不需要指针类型转换 (?) 
	u8 checksum = 0, *p = (u8 *)table;

	for (i = 0; i < table->length; ++i)
		checksum += p[i];
	if (checksum != 0) {
		/* ACPI table corrupt */
		he_err(FW_BUG "DMAR invalid checksum\n");
		return -ENODEV;
	}
	return 0;
}

/// false: 失败；true: 成功
/// 它与常规的 int 返回值，且 0 为正确 1/-1 为异常不同，
/// 一种感觉不太好的习惯，不能够支持足够丰富的错误字段内容
/// 尽管有调用 `he_err` 输出错误信息，但是这对调用者来说并不可见
bool parse_dmar(struct iommu_info *iommu_info)
{
	struct acpi_table_header *dmar_base;
	acpi_status status;
	u8 *p, *end;
	struct dmar_header *h;
	struct rmrr_header *r;
	int count_iommu = 0, count_rmrr = 0;
	int i;
	bool ret = false;
	const char *err;
	u8 *mmio_base;
	u64 capability, extended_capability, fro, nfr, iro;
	/* fault recording offset, number of fault recording, iotlb register offset. */
	u64 mmio_upper, mmio_size;

	status = acpi_get_table("DMAR", 0, &dmar_base);

	if (status == AE_NOT_FOUND) {
		he_err("DMAR acpi_table AE_NOT_FOUND\n");
		goto out;
	} else if (ACPI_FAILURE(status)) {
		err = acpi_format_exception(status);
		he_err("DMAR table: %s\n", err);
		goto out;
	}

	// 检查当前 dmar 表头信息
	if (check_dmar_checksum(dmar_base)) {
		he_err("DMAR checksum wrong\n");
		goto out;
	}

	p = (u8 *)dmar_base;
	end = (u8 *)dmar_base;
	p += DMAR_HEADER_LENGTH;
	end += dmar_base->length;
	while (p < end) {
		// 激进的 type puning... 
		// 需要代码修复
		h = (struct dmar_header *)p;

		/* Check. */
		if (h->type == DRHD_TYPE) {
			// DMA 重映射硬件单元

			// 安全检查，保证当前 count_iommu 小于上限
			if (count_iommu >= HYPERENCLAVE_MAX_IOMMU_UNITS) {
				// 越界！
				he_err("IOMMU too many\n"); 
				goto out; 
			}

			iommu_info->iommu_units[count_iommu].base = h->address;
			// 简单映射一个页，重点是取出其对应的一些结构（根据某种规则 emmm 
			// 以便之后可以决定到底映射多少长度来完成相关的 MMIO 操作
			// 但是为什么所谓的 h->address 能够得到它的物理地址呢？
			mmio_base =
				ioremap(h->address, /* Pass MEM_MAP into it. */
					SZ_4K);
			/* Read and determine mmio size. */
			if (!mmio_base) {
				he_err("Fail to map iommu control registers\n");
				goto out;
			} else {

				// 这里可以适当考虑使用位域 （除非怕什么大端序小端序 www）
				// 但直观地看，这段代码也没有处理大小端序的问题呀！
				// 除非 `read` 调用处理了这个问题

				/*
				 * Get the IOTLB & Fault Recording register offset &
				 * calc the MMIO mapping size.
				 */
				// question: 为什么 mmio_base + 0x8 / 0x10, 但没有 mmio_base 本身？
				// 相关结构内容请求，注释认为这是 IOTLB 的内容 (?) 
				capability = readq(mmio_base + 0x8);
				extended_capability = readq(mmio_base + 0x10);

				/* Fault recording offset, [33:24],10bit. */
				fro = 16 * (capability >> 24) & ((1 << 10) - 1);

				/* Number of fault recording, [47:40],8bit. */
				nfr = 1 + ((capability >> 40) & ((1 << 8) - 1));

				/* IOTLB register offset, [17:8],10bit. */
				iro = 16 * (extended_capability >> 8) &
				      ((1 << 10) - 1);

				mmio_upper = fro + nfr * FAULT_RECORDING_SIZE;

				// mmio_upper = max ( fro + nfr * FAULT... , iro + 16 ) ... 
				/* 16: size of IOTLB registers(16 bytes). */
				if (iro + 16 > mmio_upper) {
					mmio_upper = iro + 16;
				}

				// mmio_size: 
				// 2 的幂
				// 4M or 超过 mmio_upper 
				// todo: 为什么不快速计算 mmio_upper 之上的 2 的幂次. 
				// 这个应该是有指令集支持的，而不是在这里辛辛苦苦地写循环
				// question: 为什么 mmio_size 必须是 2 的幂? 感觉只要是 4K 的某倍就行了吧（页大小
				// 这里的处理思路是不是因为求快从简了？
				mmio_size = SZ_4K;
				while (mmio_size < mmio_upper &&
				       mmio_size < SZ_4M) {
					mmio_size = mmio_size * 2;
				}

				// question: `size` 对应的类型是 u32, 而 
				// `mmio_size` 类型是 u64. 
				// 这里没有证明它确实是在 u32 的范围上？
				iommu_info->iommu_units[count_iommu].size =
					mmio_size;

			}
			iounmap(mmio_base);

			++count_iommu;
		} else if (h->type == RMRR_TYPE) {
			// 保留内存区域报告结构 
			// 通常是为 DMA 保留

			// 检查越界，同上
			if (count_rmrr >= HYPERENCLAVE_MAX_RMRR_RANGES) {
				he_err("RMRR too many\n"); 
				goto out; 
			}

			/* Pass some RMRR information to hypervisor? */
			r = (struct rmrr_header *)p;
			iommu_info->rmrr_ranges[count_rmrr].base =
				r->base_address;
			iommu_info->rmrr_ranges[count_rmrr].limit =
				r->limit_address;
			++count_rmrr;
		}
		p += h->length;
	}

	// 处理剩余的 iommu info 结构，即把这些 handle 置为无效（全零）
	// 这是因为它没有实际的 iommu info count 字段，用于快速描述有效长度，
	// 只能通过将剩余 handle 置成非法值以便运行时进行实时检测
	// 此外，尽管在初始化的过程中，它的有效值总是是连续的起始一段，
	// 但实际上它也可以被实现为离散的若干值，因为所有 handle 的有效性都由自身决定，
	// 其随时可以取消自己的有效性. 
	for (i = count_rmrr; i < HYPERENCLAVE_MAX_RMRR_RANGES; ++i) {
		iommu_info->rmrr_ranges[i].base = 0;
		iommu_info->rmrr_ranges[i].limit = 0;
	}
	for (i = count_iommu; i < HYPERENCLAVE_MAX_IOMMU_UNITS; ++i) {
		iommu_info->iommu_units[i].base = 0;
		iommu_info->iommu_units[i].size = 0;
	}
	ret = true;

out:
	acpi_put_table(dmar_base);
	return ret;
}
