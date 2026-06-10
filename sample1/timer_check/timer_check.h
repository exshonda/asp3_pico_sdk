/*
 *  タイマ精度定量検証プログラム（タスク検証用）
 *  TIMER0 ALARM0 vs pico-sdk alarm pool 競合評価
 */

#ifndef TOPPERS_TIMER_CHECK_H
#define TOPPERS_TIMER_CHECK_H

#define MID_PRIORITY    4
#define STACK_SIZE      4096

#ifndef TOPPERS_MACRO_ONLY

extern void timer_check_task(intptr_t exinf);

#endif /* TOPPERS_MACRO_ONLY */

#endif /* TOPPERS_TIMER_CHECK_H */
