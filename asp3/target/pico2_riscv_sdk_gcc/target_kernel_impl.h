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
 *  kernel_impl.hのターゲット依存部の位置付けとなす．
 */

#ifndef TOPPERS_TARGET_KERNEL_IMPL_H
#define TOPPERS_TARGET_KERNEL_IMPL_H

#include "rpi_pico.h"

/*
 *  TBITW_IPRI の定義のため読み込み
 */
#include <sil.h>

/*
 *  デフォルトの非タスクコンテキスト用のスタック領域の定義
 */
#define DEFAULT_ISTKSZ    (0x1000) /* 4KByte */

#ifndef TOPPERS_MACRO_ONLY

/*
 *  ターゲットシステム依存の初期化
 */
extern void target_initialize(void);

/*
 *  ターゲットシステムの終了
 */
extern void target_exit(void) NoReturn;

/*
 *  エラー発生時の処理
 */
extern void Error_Handler(void);

#endif /* TOPPERS_MACRO_ONLY */

/*
 *  チップ依存モジュール（Xh3irq）
 */
#include <chip_kernel_impl.h>

#endif /* TOPPERS_TARGET_KERNEL_IMPL_H */
