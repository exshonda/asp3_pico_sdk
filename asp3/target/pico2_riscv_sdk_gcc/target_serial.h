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
 *  シリアルインタフェースドライバのターゲット依存部（非TECS版専用）
 *  RaspberryPi Pico2 RISC-V ASP3/pico-sdk統合版
 *  pico-sdk stdio 経由で SIO を実装する（TOPPERS_OMIT_CHIP_SERIAL 前提）
 */

#ifndef TOPPERS_TARGET_SERIAL_H
#define TOPPERS_TARGET_SERIAL_H

#include "rpi_pico.h"

/*
 *  UART割込み番号（ASP3の割込み番号 = RP2350のIRQ番号 + 1）
 *  pico-sdk stdioを使うためこの値は実際には使用されない
 */
#define USART_INTNO  (RP2350_UART0_IRQn + 1)
#define USART_INTPRI (TMAX_INTPRI - 1)
#define USART_ISRPRI 1

#define BPS_SETTING  (115200)

/*
 *  コールバックルーチンの種別（rp2350_uart.h の定義と合わせる）
 */
#define SIO_RDY_SND		1U		/* 送信可能コールバック */
#define SIO_RDY_RCV		2U		/* 受信通知コールバック */

#ifndef TOPPERS_MACRO_ONLY

typedef struct sio_port_control_block    SIOPCB;

extern void sio_initialize(EXINF exinf);
extern void sio_terminate(EXINF exinf);
extern void sio_handler(void *ptr);
extern SIOPCB *sio_opn_por(ID siopid, EXINF exinf);
extern void sio_cls_por(SIOPCB *p_siopcb);
extern bool_t sio_snd_chr(SIOPCB *p_siopcb, char c);
extern int_t sio_rcv_chr(SIOPCB *p_siopcb);
extern void sio_ena_cbr(SIOPCB *p_siopcb, uint_t cbrtn);
extern void sio_dis_cbr(SIOPCB *p_siopcb, uint_t cbrtn);
extern void sio_irdy_snd(EXINF exinf);
extern void sio_irdy_rcv(EXINF exinf);

#endif /* TOPPERS_MACRO_ONLY */
#endif /* TOPPERS_TARGET_SERIAL_H */
