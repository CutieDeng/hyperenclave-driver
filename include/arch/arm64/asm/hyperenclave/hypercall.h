/* SPDX-License-Identifier: GPL-2.0 */
/*
 * HyperEnclave kernel module.
 *
 * Copyright (C) 2020-2023 The HyperEnclave Project. All rights reserved.
 */

#pragma once 

#define hypercall_ret_N(inputs...)                                    \
	({                                                            \
		int ret;                                              \
		asm volatile("cmp %[use_hvc], #1\n\t"                 \
			     "bne 2f\n\t"                             \
			     "1:\n"                                   \
			     "   hvc #0\n\t"                         \
			     "   b 3f\n\t"                           \
			     "2:\n"                                   \
			     "   smc #0\n\t"                         \
			     "3:\n"                                   \
			     ".section .fixup,\"ax\"\n"               \
			     "4:\n"                                   \
			     "   b 3b\n"                             \
			     ".previous\n"                           \
			     : "=r"(ret)                              \
			     : [use_hvc] "r"(use_vmcall), inputs         \
			     : "memory", "cc");                       \
		ret;                                                  \
	})

#define hypercall_ret_0(x0) ({ hypercall_ret_N("0"(x0)); })

#define hypercall_ret_1(x0, x1) ({ hypercall_ret_N("0"(x0), "r"(x1)); })

#define hypercall_ret_2(x0, x1, x2) \
	({ hypercall_ret_N("0"(x0), "r"(x1), "r"(x2)); })

#define hypercall_ret_3(x0, x1, x2, x3) \
	({ hypercall_ret_N("0"(x0), "r"(x1), "r"(x2), "r"(x3)); })

/**
 * This variable selects the x86 hypercall instruction to be used by
 * hypercall_ret_N(). A caller should define and initialize the variable
 * before calling any of these functions.
 *
 * @li @c false Use AMD's VMMCALL.
 * @li @c true Use Intel's VMCALL.
 */
extern bool use_vmcall;
