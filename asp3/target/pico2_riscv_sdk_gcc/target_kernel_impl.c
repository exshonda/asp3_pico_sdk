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
 *  ターゲット依存モジュール（RaspberryPi Pico2 RISC-V ASP3/pico-sdk統合版）
 *
 *  ベアメタル版（pico2_riscv_gcc）との主な違い：
 *  - hardware_init_hook/software_init_hook は不要（pico-sdkのruntime_init()
 *    がクロック・PLL・TIMER0 TICKS等の初期化を行う）
 *  - シリアルは pico-sdk stdio 経由（target_serial.c）
 *  - target_initialize() は chip_initialize() を呼ぶだけ
 *    （chip_initialize() が mtvec を trap_vector_table に設定し，
 *     Xh3irq を初期化する → 以降の割込みは ASP3 が掌握）
 */

#include "kernel_impl.h"
#include "target_syssvc.h"
#include <sil.h>

extern void Error_Handler(void);

/*
 *  ターゲット依存部 初期化処理
 *
 *  chip_initialize() が mtvec を ASP3 の trap_vector_table に向け，
 *  Xh3irq の初期化を行う．pico-sdk startup が既に設定した mtvec を
 *  上書きするが，sta_ker() 呼出し時点では割込みが来ないため安全．
 */
void
target_initialize(void)
{
	chip_initialize();
}

/*
 *  ターゲット依存部 終了処理
 */
void
target_exit(void)
{
	chip_terminate();
	while (1) ;
}

/*
 *  エラー発生時の処理
 */
void
Error_Handler(void)
{
	while (1) ;
}

__attribute__((weak))
void software_term_hook(void)
{
}
