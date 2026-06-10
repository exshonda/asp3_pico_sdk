/*
 *  シリアルインタフェースドライバのターゲット依存部（非TECS版専用）
 *  RaspberryPi Pico 2 ASP3用
 *  pico stdio を使用して SIO を実装する．
 */

#include <stdint.h>
#include <stdio.h>
#include "t_stddef.h"
#include "target_serial.h"
#include "target_syssvc.h"
#include "pico/stdio.h"

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

/*
 *  SIOドライバの初期化
 */
void sio_initialize(EXINF exinf)
{
    for (uint_t i = 0; i < TNUM_PORT; i++) {
        siopcb_table[i].exinf = (intptr_t)exinf;
        siopcb_table[i].handle = NULL;
    }
}

/*
 *  SIOドライバの終了処理
 */
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

/*
 *  SIOポートのオープン
 */
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

    stdio_set_chars_available_callback(p_siopcb->handle, p_siopcb);

    if (p_siopcb->rdy_snd) {
        sio_irdy_snd(p_siopcb->exinf);
    }

    return p_siopcb;
}

/*
 *  SIOポートのクローズ
 */
void sio_cls_por(SIOPCB *p_siopcb)
{
    stdio_set_chars_available_callback(NULL, NULL);
    p_siopcb->handle = NULL;
}

/*
 *  SIOポートへの文字送信
 */
bool_t sio_snd_chr(SIOPCB *p_siopcb, char ch)
{
    p_siopcb->snd_buf[p_siopcb->snd_wpos] = ch;
    if (putchar_raw(p_siopcb->snd_buf[p_siopcb->snd_wpos]) == ch) {
        p_siopcb->snd_wpos = (p_siopcb->snd_wpos + 1) % sizeof(p_siopcb->snd_buf);
        return true;
    }
    return false;
}

/*
 *  SIOポートからの文字受信
 */
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

/*
 *  SIOポートからのコールバックの許可
 */
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

/*
 *  SIOポートからのコールバックの禁止
 */
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

/*
 *  SIOポートへの文字出力（低レベル出力）
 */
void target_fput_log(char c)
{
    if (c == '\n') {
        putc('\r', stdout);
    }
    putc(c, stdout);
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
