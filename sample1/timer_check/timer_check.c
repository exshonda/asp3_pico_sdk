/*
 *  タイマ精度定量検証プログラム
 *
 *  【検証内容】
 *  1. dly_tsk(1000000) 精度 — 10 試行の fch_hrt / get_tim delta を計測
 *  2. get_tim() 単調増加 + ドリフト — 5 分（300 秒）1 秒ごとにサンプリング
 *
 *  【タイマ競合の前提知識（静的解析結果）】
 *   - ASP3 HRT : TIMER0 ALARM0 (RP2350_TIMER0_ALARM0)  IRQ=TIMER0_0
 *   - pico-sdk  : TIMER0 ALARM3 (PICO_TIME_DEFAULT_ALARM_POOL_HARDWARE_ALARM_NUM=3)
 *   → 使用チャンネルが異なるため直接競合なし
 *   → busy_wait_* / time_us_64() は TIMER0 TIMERAWL/H 直読み（書込みなし）
 *
 *  シリアル出力: UART0 / GP0(TX) GP1(RX) / 115200 8N1
 */

#include "kernel.h"
#include "syssvc/syslog.h"
#include "timer_check.h"

#define DELY_US     1000000UL   /* 1 秒 */
#define N_ACCURACY  10          /* dly_tsk 精度計測：試行数 */
#define N_MONO      300         /* 単調増加検証：サンプル数（秒） */

void
timer_check_task(intptr_t exinf)
{
    HRTCNT h0, h1;
    SYSTIM s0, s1;

    syslog(LOG_NOTICE, "=== TIMER CHECK START ===");
    syslog(LOG_NOTICE, "ASP3 HRT: TIMER0 ALARM0  SDK default pool: TIMER0 ALARM3");
    syslog(LOG_NOTICE, "");

    /* dly_tsk(1000000) 精度計測 */
    syslog(LOG_NOTICE, "--- dly_tsk(1000000) accuracy (%d trials) ---", N_ACCURACY);
    for (int i = 0; i < N_ACCURACY; i++) {
        h0 = fch_hrt();
        get_tim(&s0);
        dly_tsk(DELY_US);
        h1 = fch_hrt();
        get_tim(&s1);
        /* fch_hrt は 32bit HRTCNT（差分演算で自然にラップ処理） */
        uint32_t dh = (uint32_t)(h1 - h0);
        uint32_t ds = (uint32_t)(s1 - s0);
        /* 誤差 = (実測 - 設計) us */
        int32_t err_h = (int32_t)dh - (int32_t)DELY_US;
        int32_t err_s = (int32_t)ds - (int32_t)DELY_US;
        syslog(LOG_NOTICE, "[%2d] fch_hrt=%lu us (err=%ld)  get_tim=%lu us (err=%ld)",
               i, (unsigned long)dh, (long)err_h,
                  (unsigned long)ds, (long)err_s);
    }

    syslog(LOG_NOTICE, "");

    /* get_tim() 単調増加 + ドリフト検証 */
    syslog(LOG_NOTICE, "--- monotone + drift check (%d s) ---", N_MONO);

    HRTCNT prev_h = fch_hrt();
    SYSTIM prev_s;
    get_tim(&prev_s);
    SYSTIM mono_start = prev_s;  /* 単調チェック開始時刻 */
    int errors = 0;

    for (int i = 1; i <= N_MONO; i++) {
        dly_tsk(DELY_US);
        HRTCNT now_h  = fch_hrt();
        SYSTIM now_s;
        get_tim(&now_s);

        uint32_t dh = (uint32_t)(now_h - prev_h);
        uint32_t ds = (uint32_t)(now_s - prev_s);  /* 1 秒区間、32bit で十分 */

        if (now_s <= prev_s) {
            errors++;
            syslog(LOG_ERROR, "[%3ds] ERROR: get_tim not monotone!", i);
        }

        if (i % 10 == 0) {
            /* 累積ドリフト（us 単位、設計値 = i * 1000000） */
            int32_t drift = (int32_t)((uint32_t)(now_s - mono_start) - (uint32_t)(i * DELY_US));
            syslog(LOG_NOTICE, "[%3ds] fch_hrt_d=%lu  get_tim_d=%lu  cumul_drift=%ld us",
                   i, (unsigned long)dh, (unsigned long)ds, (long)drift);
        }

        prev_h = now_h;
        prev_s = now_s;
    }

    syslog(LOG_NOTICE, "");
    syslog(LOG_NOTICE, "=== TIMER CHECK DONE (errors=%d) ===", errors);

    ext_ker();
}
