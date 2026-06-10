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
 *  システムサービスのターゲット依存部
 *  （RaspberryPi Pico2 RISC-V ASP3/pico-sdk統合版）
 */

#ifndef TOPPERS_TARGET_SYSSVC_H
#define TOPPERS_TARGET_SYSSVC_H

#ifdef TOPPERS_OMIT_TECS

/*
 *  起動メッセージのターゲットシステム名
 */
#define TARGET_NAME "RaspberryPi Pico2 <RISC-V Hazard3>"

/*
 *  システムログの低レベル出力のための文字出力
 */
extern void target_fput_log(char c);

/*
 *  低レベル出力で使用するSIOポートID
 */
#define SIOPID_FPUT 1

/*
 *  シリアルポート数
 */
#define TNUM_PORT   1

#endif /* TOPPERS_OMIT_TECS */

/*
 *  ターゲット固有定数（CORE_CLK_MHZ 等）
 */
#include "rpi_pico.h"

/*
 *  コアで共通な定義
 */
#include "core_syssvc.h"

#endif /* TOPPERS_TARGET_SYSSVC_H */
