/* This file is generated from target_rename.def by genrename. */

#ifndef TOPPERS_TARGET_RENAME_H
#define TOPPERS_TARGET_RENAME_H

/*
 *  target_config.c
 */
#define target_initialize   _kernel_target_initialize
#define target_exit         _kernel_target_exit

#ifdef TOPPERS_LABEL_ASM
#define _target_initialize  __kernel_target_initialize
#define _target_exit        __kernel_target_exit
#endif /* TOPPERS_LABEL_ASM */

#include "chip_rename.h"

/*
 *  pico-sdk統合のため，アセンブリエントリポイントをpico-sdkのISR名にリダイレクト
 *  core_exc_entry → isr_invalid (pico-sdkのWEAKシンボルをオーバーライド)
 *  svc_handler    → isr_svcall
 *  pendsv_handler → isr_pendsv
 */
#undef core_exc_entry
#define core_exc_entry      isr_invalid
#undef svc_handler
#define svc_handler         isr_svcall
#undef pendsv_handler
#define pendsv_handler      isr_pendsv

#endif /* TOPPERS_TARGET_RENAME_H */
