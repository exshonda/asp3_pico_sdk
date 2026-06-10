/*
 *  TOPPERS Software
 *      Toyohashi Open Platform for Embedded Real-Time Systems
 *
 *  Copyright (C) 2026 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *
 *  上記著作権者は，本ソフトウェアをTOPPERSライセンス（条件は他のソー
 *  スファイルの先頭コメントを参照）の下で利用することを許諾する．本ソ
 *  フトウェアは無保証で提供される．
 */

/*
 *  RaspberryPi Pico2（RISC-V）ボード依存の定義
 */

#ifndef TOPPERS_RPI_PICO_H
#define TOPPERS_RPI_PICO_H

/*
 *  コアクロック周波数（clk_sys = 150MHz，pico-sdk が設定）
 */
#define CPU_CLOCK_HZ    150000000
#define CORE_CLK_MHZ    150

/*
 *  微少時間待ちのための定義（sil_dly_nse 用）
 *  実測値（2026-06-06）: 呼出46ns，1ループ13.3〜20.5ns @ 150MHz
 */
#define SIL_DLY_TIM1    40
#define SIL_DLY_TIM2    13

#endif /* TOPPERS_RPI_PICO_H */
