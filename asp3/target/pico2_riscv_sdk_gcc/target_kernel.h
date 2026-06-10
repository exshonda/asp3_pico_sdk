/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2026 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *
 *  上記著作権者は，本ソフトウェアをTOPPERSライセンス（条件は他のソー
 *  スファイルの先頭コメントを参照）の下で利用することを許諾する．本ソ
 *  フトウェアは無保証で提供される．
 */

/*
 *  kernel.hのターゲット依存部
 *  （RaspberryPi Pico2 RISC-V ASP3/pico-sdk統合版）
 *
 *  このインクルードファイルは，kernel.hでインクルードされる．
 */

#ifndef TOPPERS_TARGET_KERNEL_H
#define TOPPERS_TARGET_KERNEL_H

#include "rpi_pico.h"

#ifdef USE_TIM_AS_HRT

/*
 *  高分解能タイマのタイマ周期
 *  タイマ周期が2^32の場合には，このマクロを定義しない
 */
#undef TCYC_HRTCNT

/*
 *  高分解能タイマのカウント値の進み幅
 */
#define TSTEP_HRTCNT 1U

#endif /* USE_TIM_AS_HRT */

/*
 *  チップで共通な定義
 */
#include "chip_kernel.h"

#ifndef TOPPERS_MACRO_ONLY

extern void sta_ker(void);

#endif /* TOPPERS_MACRO_ONLY */

#endif /* TOPPERS_TARGET_KERNEL_H */
