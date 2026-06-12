# docs/ — asp3_pico_sdk 設計・経緯の記録

本リポジトリ（TOPPERS/ASP3 Core × Raspberry Pi Pico SDK 協調動作）の
**設計判断・実測データ・経緯**を残す。使い方・ビルド手順はルートの
[README.md](../README.md)、AI 向け作業指示は [CLAUDE.md](../CLAUDE.md) を参照。

| ファイル | 内容 |
|---|---|
| [design.md](design.md) | SDK協調の設計：割込み共存（ARM `--wrap`／RISC-V の mtvec 非互換と ASP3ネイティブISR）、ライブラリ専用モード、ツールチェーン、pico-sdk バージョン互換 |
| [timer-coexistence.md](timer-coexistence.md) | タイマ競合（SDK alarm vs ASP3 HRT）の解析と PICO2 実測結果・アプリ側の制約 |
| [history.md](history.md) | リポジトリ再編（fork廃止→submodule化）の経緯・検証マトリクス・残課題 |

## 他リポジトリの関連記録（役割分担）

- **統合全体の経緯・asp3_core 側の受け入れ口**（`ASP3_TARGET_DIR`／`ASP3_LIBRARY_ONLY`）：
  `asp3/asp3_core/docs/dev/pico-sdk-integration.md`（正本）
- **外部（SDK）ターゲットの書き方の規約**：`asp3/asp3_core/docs/porting/PORTING_GUIDE.md`
- **SDK非依存ベアメタル版**（pico2_arm_gcc / pico2_riscv_gcc）：asp3_core 本体の `target/`
  （本リポジトリの移植元。RISC-V の Xh3irq 実装の経緯は同 `docs/dev/pico2-riscv.md`）

> 原則：**カーネル・共通archに関わる記録は asp3_core 側**、**pico-sdk との共存に
> 固有の記録は本リポジトリ（ここ）**に置く。
