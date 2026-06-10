# -*- coding: utf-8 -*-
#
#		パス2の生成スクリプトのターゲット依存部
#		（RaspberryPi Pico2 RISC-V ASP3/pico-sdk統合版）
#

#
#  生成スクリプトのチップ依存部（INTNO_VALID 等＋ core_kernel.py 経由で
#  kernel/kernel.py をインクルードする）
#
IncludeTrb("chip_kernel.py")

#
#  OMIT_ISTACKが定義されている場合，kernel.pyは_kernel_istk/_kernel_istkptを
#  生成しないが，core_support.S（call_exit_kernel・dispatcher・割込みエントリ等）
#  は常に istkpt（=_kernel_istkpt）を参照する．
#  ARM-S版（pico2_arm_sdk_gcc/target_kernel.py）と同一の対処．
#
if OMIT_ISTACK:
    kernelCfgC.comment_header("Stack Area for Non-task Context (pico-sdk integration)")
    kernelCfgC.add("static STK_T _kernel_istack[COUNT_STK_T(DEFAULT_ISTKSZ)];")
    kernelCfgC.add("const size_t _kernel_istksz = ROUND_STK_T(DEFAULT_ISTKSZ);")
    kernelCfgC.add("STK_T *const _kernel_istk = _kernel_istack;")
    kernelCfgC.add("#ifdef TOPPERS_ISTKPT")
    kernelCfgC.add("STK_T *const _kernel_istkpt = TOPPERS_ISTKPT(_kernel_istack, ROUND_STK_T(DEFAULT_ISTKSZ));")
    kernelCfgC.add("#endif /* TOPPERS_ISTKPT */")
    kernelCfgC.add()
