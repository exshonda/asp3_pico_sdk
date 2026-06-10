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
 *  pico-sdk stdio を使用して SIO を実装する．
 *  ARM-S版（pico2_arm_sdk_gcc/target_serial.c）と同一実装（ISA非依存）
 */

#include <stdint.h>
#include <stdio.h>
#include "t_stddef.h"
#include "target_serial.h"
#include "target_syssvc.h"
#include "pico/stdio.h"
#include "hardware/structs/uart.h"	/* uart0_hw：RX 割込み（ASP3 ネイティブ ISR）で UART0 を直接操作 */

extern void sio_irdy_snd(intptr_t exinf);

struct sio_port_control_block
{
	void (*handle)(void*);
	intptr_t exinf;
	bool_t rdy_snd;
	uint8_t snd_buf[256];
	uint32_t snd_wpos;
	uint32_t snd_rpos;
	bool_t rdy_rcv;
	uint8_t rcv_buf[256];
	uint32_t rcv_wpos;
	uint32_t rcv_rpos;
};

static SIOPCB siopcb_table[TNUM_PORT] = {
	{NULL, 0, false, {0}, 0, 0, false, {0}, 0, 0},
};

#define INDEX_SIOP(siopid)  ((uint_t)((siopid) - 1))
#define get_siopcb(siopid)  (&(siopcb_table[INDEX_SIOP(siopid)]))

void sio_initialize(EXINF exinf)
{
	for (uint_t i = 0; i < TNUM_PORT; i++) {
		siopcb_table[i].exinf = (intptr_t)exinf;
		siopcb_table[i].handle = NULL;
	}
}

void sio_terminate(EXINF exinf)
{
	uint_t i;
	SIOPCB *p_siopcb;

	for (i = 0; i < TNUM_PORT; i++) {
		p_siopcb = &(siopcb_table[i]);
		if (p_siopcb->handle) {
			sio_cls_por(&(siopcb_table[i]));
		}
	}
}

SIOPCB *sio_opn_por(ID siopid, EXINF exinf)
{
	SIOPCB *p_siopcb = NULL;
	if (siopid > TNUM_PORT) {
		return p_siopcb;
	}

	p_siopcb = get_siopcb(siopid);
	if (p_siopcb->handle != NULL) {
		return NULL;
	}

	p_siopcb->exinf = (intptr_t)exinf;
	p_siopcb->handle = sio_handler;
	p_siopcb->rcv_wpos = 0;
	p_siopcb->rcv_rpos = 0;

	/*
	 *  受信割込み（RX）の有効化．
	 *  RISC-V（Hazard3/Xh3irq）では pico-sdk の RX 経路
	 *  （stdio_set_chars_available_callback→irq_set_exclusive_handler）は使えない．
	 *  pico-sdk は mtvec を「IRQ番号で索引する関数ポインタ表」とみなすが，ASP3 は
	 *  mtvec を VECTORED モードの cause 索引 jump 表に使い，外部割込みは meinext で
	 *  Xh3irq デマルチプレクスして const の _kernel_inh_table へ振るため，両者の
	 *  mtvec 解釈が非互換（pico-sdk の hard_assert で PANIC する）．
	 *  そこで RX は ASP3 ネイティブで受ける：UART0 の RX 割込みを有効化し，
	 *  ハンドラ（sio_isr）は cfg の CRE_ISR で ASP3 の割込み管理に登録する
	 *  （UART ペリフェラルの IMSC 設定のみ pico-sdk uart API で行い，irq 登録は
	 *   一切呼ばない＝衝突なし）．TX は従来どおり pico-sdk stdio（putchar_raw）．
	 */
#if defined(__riscv)
	/*  UART0 の受信割込み（RXIM）と受信タイムアウト割込み（RTIM）を許可  */
	uart0_hw->imsc |= UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS;
#else
	stdio_set_chars_available_callback(p_siopcb->handle, p_siopcb);
#endif

	if (p_siopcb->rdy_snd) {
		sio_irdy_snd(p_siopcb->exinf);
	}

	return p_siopcb;
}

void sio_cls_por(SIOPCB *p_siopcb)
{
#if !defined(__riscv)
	stdio_set_chars_available_callback(NULL, NULL);
#endif
	p_siopcb->handle = NULL;
}

bool_t sio_snd_chr(SIOPCB *p_siopcb, char ch)
{
	p_siopcb->snd_buf[p_siopcb->snd_wpos] = ch;
	if (putchar_raw(p_siopcb->snd_buf[p_siopcb->snd_wpos]) == ch) {
		p_siopcb->snd_wpos = (p_siopcb->snd_wpos + 1) % sizeof(p_siopcb->snd_buf);
		return true;
	}
	return false;
}

int_t sio_rcv_chr(SIOPCB *p_siopcb)
{
	uint8_t ch;
	if (p_siopcb->rcv_wpos != p_siopcb->rcv_rpos) {
		ch = p_siopcb->rcv_buf[p_siopcb->rcv_rpos];
		p_siopcb->rcv_rpos = (p_siopcb->rcv_rpos + 1) % sizeof(p_siopcb->rcv_buf);
		return ch;
	}
	return -1;
}

void sio_ena_cbr(SIOPCB *p_siopcb, uint_t cbrtn)
{
	switch (cbrtn) {
	case SIO_RDY_SND:
		p_siopcb->rdy_snd = true;
		sio_irdy_snd(p_siopcb->exinf);
		break;
	case SIO_RDY_RCV:
		p_siopcb->rdy_rcv = true;
		break;
	default:
		break;
	}
}

void sio_dis_cbr(SIOPCB *p_siopcb, uint_t cbrtn)
{
	switch (cbrtn) {
	case SIO_RDY_SND:
		p_siopcb->rdy_snd = false;
		break;
	case SIO_RDY_RCV:
		p_siopcb->rdy_rcv = false;
		break;
	default:
		break;
	}
}

void target_fput_log(char c)
{
	if (c == '\n') {
		putc('\r', stdout);
	}
	putc(c, stdout);
}

/*
 *  UART 受信割込みサービスルーチン（ASP3 ネイティブ・RISC-V）
 *  cfg の CRE_ISR で UART0 の割込み番号に結び付けられる．
 *  pico-sdk の irq 登録は使わず，UART ペリフェラルから直接受信する．
 */
void sio_isr(intptr_t exinf)
{
	SIOPCB *p_siopcb = &siopcb_table[0];	/* TNUM_PORT==1 */

	/*  RX FIFO を空になるまで吸い出す（FIFO ドレインで RXI はクリアされる）  */
	while ((uart0_hw->fr & UART_UARTFR_RXFE_BITS) == 0U) {
		p_siopcb->rcv_buf[p_siopcb->rcv_wpos] = (uint8_t)(uart0_hw->dr & 0xffU);
		p_siopcb->rcv_wpos = (p_siopcb->rcv_wpos + 1) % sizeof(p_siopcb->rcv_buf);
	}
	/*  受信／受信タイムアウト割込みをクリア  */
	uart0_hw->icr = UART_UARTICR_RXIC_BITS | UART_UARTICR_RTIC_BITS;

	if (p_siopcb->rdy_rcv) {
		sio_irdy_rcv(p_siopcb->exinf);
	}
}

void sio_handler(void *ptr)
{
	SIOPCB *p_siopcb = NULL;
	for (uint_t i = 0; i < TNUM_PORT; i++) {
		if (siopcb_table[i].handle == sio_handler) {
			p_siopcb = &siopcb_table[i];
			break;
		}
	}

	if (p_siopcb == NULL) {
		return;
	}

	p_siopcb->rcv_buf[p_siopcb->rcv_wpos] = stdio_getchar_timeout_us(0);
	p_siopcb->rcv_wpos = (p_siopcb->rcv_wpos + 1) % sizeof(p_siopcb->rcv_buf);

	if (p_siopcb->rdy_rcv) {
		sio_irdy_rcv(p_siopcb->exinf);
	}
}
