# タイマ競合の解析と実測（SDK alarm vs ASP3 HRT）

pico-sdk のタイマ機能（alarm／`sleep_ms` 等）と ASP3 の高分解能タイマ（HRT）が
同じ **TIMER0** を使うことによる競合の解析・PICO2 実機での定量検証の記録（2026-06-11）。

## 結論

**pico-sdk のタイマ API（`sleep_ms` / `add_alarm_*` 等）は現状のまま併用可能**。
ALARM チャンネルが分離されており、ASP3 カーネル側・pico-sdk 側とも変更不要。

## 静的解析

| コンポーネント | ハードウェア | アラームチャンネル | IRQ |
|---|---|---|---|
| ASP3 HRT（`USE_TIM_AS_HRT`） | TIMER0 | **ALARM0**（`RP2350_TIMER0_ALARM0`） | `TIMER0_0` |
| pico-sdk 既定 alarm pool | TIMER0 | **ALARM3**（`PICO_TIME_DEFAULT_ALARM_POOL_HARDWARE_ALARM_NUM=3`） | `TIMER0_3` |

- チャンネル分離により**直接競合なし**。
- `busy_wait_us()`／`time_us_64()` は TIMERAWL/H の**読み取りのみ**（書込みなし）→安全。
- alarm pool 初期化時の `irq_set_exclusive_handler(TIMER0_3, …)` は ARM の `--wrap` 経由で
  ASP3 の `_kernel_exc_tbl` に正しく登録される。

## PICO2 実測（ARM-S・検証プログラム `sample1/timer_check/`）

**dly_tsk(1000000) 精度（10試行）**

| 試行 | fch_hrt 実測 (us) | 誤差 | get_tim 実測 (us) | 誤差 |
|---|---|---|---|---|
| 0 | 1,000,014 | +14 | 1,000,013 | +13 |
| 1–9 | 1,000,006 | +6 | 1,000,006〜7 | +6〜7 |

初回のみ +14us（ログタスク起動オーバーヘッド）、以降は安定して **+6us（+6ppm）**。

**get_tim() 単調増加＋ドリフト（300秒）**

- 単調増加違反 **errors=0**。
- 累積ドリフト 1,934us／300s ＝ **6.45ppm**（水晶の周波数オフセット相当）。
- `fch_hrt()` と `get_tim()` の測定値が完全一致 → 同一 TIMER0 TIMERAWL からの導出を確認。

## アプリ側の制約（守ること）

1. **alarm pool に ALARM0 を指定しない**（`alarm_pool_create(timer0, 0, …)` の `0` は禁止。
   ASP3 HRT が使用）。既定 pool（ALARM3）を使う限り問題なし。
2. `busy_wait_us()`／`sleep_ms()` は**タスクコンテキストで**呼ぶ（ASP3 の割込み規約）。
3. ASP3 HRT を別タイマへ逃がす・SDK タイマ API を禁止する、等の対策は**不要**。

## 再検証の方法

`sample1/timer_check/` をビルド・書込み（タスク定義は `timer_check.cfg`）。
シリアルに dly_tsk 10試行＋300秒の単調性チェック結果が出力される。

> 残課題：RISC-V（Hazard3）側での同等の定量検証は未実施（ARM-S のみ実測済み）。
