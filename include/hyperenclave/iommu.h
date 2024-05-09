/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#pragma once 

#include <asm/hyperenclave/iommu.h>

#ifndef ARCH_STRUCT_IOMMU
#error ARCH_STRUCT_IOMMU not defined
#endif

bool parse_iommu(struct iommu_info *iommu_info);
