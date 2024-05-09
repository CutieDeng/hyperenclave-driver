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
		asm volatile("cmpb $0x1, %[use_vmcall]\n\t"           \
			     "jne 2f\n\t"                             \
			     "1:\n"                                   \
			     "   vmcall\n\t"                          \
			     "   jmp 3f\n\t"                          \
			     "2:\n"                                   \
			     "   vmmcall\n\t"                         \
			     "3:\n"                                   \
			     ".section .fixup,\"ax\"\n"               \
			     "4:\n"                                   \
			     "   jmp 3b\n"                            \
			     ".previous\n" _ASM_EXTABLE_FAULT(1b, 4b) \
				     _ASM_EXTABLE_FAULT(2b, 4b)       \
			     : "=a"(ret)                              \
			     : [use_vmcall] "m"(use_vmcall), inputs   \
			     : "memory", "cc");                       \
		ret;                                                  \
	})

// #define hypercall_ret_N(inputs...)                                    \
// 	({                                                            \
// 		int ret;                                              \
// 		asm volatile("cmp %[use_hvc], #1\n\t"                 \
// 			     "bne 2f\n\t"                             \
// 			     "1:\n"                                   \
// 			     "   hvc #0\n\t"                         \
// 			     "   b 3f\n\t"                           \
// 			     "2:\n"                                   \
// 			     "   smc #0\n\t"                         \
// 			     "3:\n"                                   \
// 			     ".section .fixup,\"ax\"\n"               \
// 			     "4:\n"                                   \
// 			     "   b 3b\n"                             \
// 			     ".previous\n" _ASM_EXTABLE_FAULT(1b, 4b) \
// 				     _ASM_EXTABLE_FAULT(2b, 4b)       \
// 			     : "=r"(ret)                              \
// 			     : [use_hvc] "r"(use_hvc), inputs         \
// 			     : "memory", "cc");                       \
// 		ret;                                                  \
// 	})


#define hypercall_ret_0(rax) ({ hypercall_ret_N("a"(rax)); })

#define hypercall_ret_1(rax, rdi) ({ hypercall_ret_N("a"(rax), "D"(rdi)); })

#define hypercall_ret_2(rax, rdi, rsi) \
	({ hypercall_ret_N("a"(rax), "D"(rdi), "S"(rsi)); })

#define hypercall_ret_3(rax, rdi, rsi, rdx) \
	({ hypercall_ret_N("a"(rax), "D"(rdi), "S"(rsi), "d"(rdx)); })

/**
 * This variable selects the x86 hypercall instruction to be used by
 * hypercall_ret_N(). A caller should define and initialize the variable
 * before calling any of these functions.
 *
 * @li @c false Use AMD's VMMCALL.
 * @li @c true Use Intel's VMCALL.
 */
extern bool use_vmcall;
