# 経緯（リポジトリ再編・検証記録・残課題）

## リポジトリ再編（2026-06-10）

| 旧 | 新 | 内容 |
|---|---|---|
| `asp3_pico_sdk`（カーネル同梱 fork） | `asp3_pico_sdk_legacy`（**archived**・読取専用） | 移植元として温存 |
| `asp3_pico_sdk_sample` | **`asp3_pico_sdk`（本リポジトリ・正本）** | 純カーネル asp3_core を submodule 参照する新構成 |

動機：SDK 固有部を asp3_core に入れず外側で持つことで asp3_core を純カーネルに保ち、
上流 ASP3 への追従（手動マージ）を軽くする。fork した CMakeLists の保守を廃止し、
asp3_core の正準 CMake（`ASP3_LIBRARY_ONLY`）＋受け入れ口（`ASP3_TARGET_DIR`）に一本化。
FSP・STM32 統合の雛形（submodule 構成の第1例）。

## 主要コミットの時系列

| コミット | 内容 |
|---|---|
| `f843f52` | **submodule 移行（A案）**：fork CMake 廃止→ `add_subdirectory(asp3_core, ASP3_LIBRARY_ONLY)`。ARM-S ビルド＋PICO2 実機 task1 確認 |
| `458cb06` | `timer_check/` 追加・**タイマ競合の定量検証**（→ [timer-coexistence.md](timer-coexistence.md)） |
| `2af62e8` | ASP3 移植部を `asp3/` 配下へ集約・tool arch を asp3_core から共有 |
| `d2fa424` | **RISC-V（Hazard3）実機ブート**。当初は Ubuntu picolibc ツールチェーン向け回避策（specs 改変等）を導入 |
| `7e2c699` | ARM も asp3_core のチップ arch を共用（SDK 側 arch 重複の全廃）。pico-sdk 提供物（start.S・chip_serial）の REMOVE_ITEM 方式を確立 |
| `5d7eace` | **RISC-V UART RX**：mtvec 解釈非互換のため `--wrap` 不成立 → ASP3 ネイティブ ISR（`CRE_ISR`）方式で実機確認（→ [design.md](design.md) §2） |
| `39600b2` | sample を単一 `sample1/` に統合（`PICO_PLATFORM` 切替）。RISC-V を拡張同梱 toolchain（newlib）に一本化し picolibc 回避策を全廃 |
| `3429413` | **pico-sdk 2.2.0 対応**（`pico_platform_common` include 追加。2.1.1 と両対応） |
| `bc69572` | 協調ヘルパを `asp3/asp3_pico_sdk.cmake` へ移動（**asp3_fsp / stm32_vscode_asp と構成統一**） |

## asp3_core 側に取り込まれた関連修正（本リポジトリ起点）

- `ASP3_TARGET_DIR`（外部ターゲット受け入れ口）・`ASP3_LIBRARY_ONLY`（ライブラリ専用モード）
  — Pico 統合のために asp3_core へ追加（`9a15203`・`5619f92`）。
- ベアメタル pico2 ターゲットの改称（`raspberrypi_pico2*_gcc` → `pico2_{arm,riscv}_gcc`）
  — SDK 側ターゲット名（`pico2_*_sdk_gcc`）との対称性（`221a13f`）。

## 検証マトリクス

| 項目 | ARM-S (Cortex-M33) | RISC-V (Hazard3) |
|---|---|---|
| ビルド（pico-sdk 2.1.1） | ✅ | ✅（拡張 RISC-V toolchain） |
| ビルド（pico-sdk 2.2.0） | ✅ | ✅ |
| PICO2 実機 task1（周期出力） | ✅ | ✅ |
| PICO2 実機 シリアル RX（act_tsk 操作） | ✅（`--wrap` 経由） | ✅（ASP3 ネイティブ ISR） |
| timer_check（タイマ精度の定量検証） | ✅（+6ppm・単調性 OK） | ⬜ 未実施 |

## 残課題

- RISC-V 側の timer_check 定量検証（ARM-S と同等の測定）。
- OS Awareness（gdb デバッグ支援）の SDK ターゲット対応（asp3_core 側の残課題と連動）。
- pico-sdk 新バージョン追従時の確認観点は [design.md](design.md) §4 を参照。
