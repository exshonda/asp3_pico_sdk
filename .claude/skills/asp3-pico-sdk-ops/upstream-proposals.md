# toppers-skills への汎用候補 提案（出所: asp3_pico_sdk）

> 本ファイルは **提案メモ**。toppers-skills は直接編集・push しない（AGENTS.md §6）。
> キュレーターが dedup・命名・実装非依存性を点検して取り込む。各項目は実装固有語
> （pico-sdk / irq_set_exclusive_handler / ターゲット名 / 番地 / パス）を本文に書かず、
> 分類・プロセッサ・出所のみメタ注記で残す方針。

既存3 skill（toppers-asp / toppers-kernel-dev / toppers-kernel-debug）と重複しないことを確認済み
（kernel-dev references＝inviolable-rules/memory-protection-porting/mp-to-sp-porting/upstream-merge、
kernel-debug references＝observing-execution/performance-measurement/host-simulation-port 等に該当なし）。

---

## 提案1：RTOS が割込みを掌握する移植での「ベンダSDK の割込み登録APIとの共存」（ARM vs RISC-V の構造差）

- **対象skill**：`toppers-kernel-dev` ／ 新 reference 例 `references/vendor-sdk-interrupt-coexistence.md`
  （porting系 references と並置）
- **追記案の要旨**：RTOS が割込みコントローラを掌握する移植で、同梱ベンダSDK が*動的に*割込み
  ハンドラを登録しようとするときの共存策はアーキで異なる。
  - **M-profile（RAM 再配置ベクタ表＝関数ポインタ表）**：SDK の割込み登録API群を
    リンカ `--wrap` で RTOS 管理へ誘導し、RTOS の RAM ベクタ表へ書ければ共存できる。
    併せて SDK 側のベクタ表設置・IRQ優先度初期化を抑止し、RTOS が制御器を掌握する。
  - **RISC-V（vectored mtvec）**：mtvec は cause 索引の*命令*表で、外部割込みは demux して
    RTOS の **const** なハンドラ表へ振る。SDK は mtvec を「IRQ番号索引の*関数ポインタ*表」と
    みなすため解釈が非互換で、SDK の動的登録API はそのまま使えない（アサート/暴走）。
    → 周辺の割込みは **RTOS のネイティブな静的ISR登録**（コンフィグ時登録）で受け、
    周辺側の割込み許可だけ行う（SDK の登録APIは呼ばない）。
- **分類**：制約/アーキ構造（＋統合/割込み共存）
- **プロセッサ**：ARM Cortex-M（M-profile）／RISC-V（vectored mtvec。例：Hazard3 系）
- **出所リポ**：asp3_pico_sdk
- **実装非依存の根拠**：固有名(SDK名/関数名/CSR値/ターゲット名)を本文に書かず、「ベンダSDK」
  「RTOSのベクタ表(RAM/const)」「vectored mtvec の cause 索引」「ネイティブ静的ISR登録」で記述。

## 提案2：RTOS のティック/高分解能タイマと、ベンダSDK の既定アラームプールが同一タイマを共有する場合のチャネル分離

- **対象skill**：`toppers-kernel-dev`（移植/タイマの注意点。短い項目で porting系へ追記）
- **追記案の要旨**：RTOS が高分解能タイマ/ティックに HWタイマの*アラーム比較器*を使い、同梱SDK の
  既定アラームプールも同じタイマの*別*アラームを使う構成では、二重 claim による起動時アサートを
  避けるため**使用アラーム（比較器）チャネルが分離**していることを確認する。実測でドリフトも見る。
- **分類**：時間分割/タイマ（＋統合）
- **プロセッサ**：非依存（複数アラーム比較器を持つ HWタイマ一般）
- **出所リポ**：asp3_pico_sdk
- **実装非依存の根拠**：固有のアラーム番号・API名を書かず「HWタイマのアラーム比較器」
  「SDK の既定アラームプール」「二重 claim」で記述。

## 提案3：ベアメタル向けSDKを picolibc系ツールチェーンでビルドする際の newlib 前提との齟齬

- **対象skill**：`toppers-kernel-dev`（移植/ツールチェーン・リンカ。新 reference か porting系へ追記）
- **追記案の要旨**：ベンダSDK のビルドは **newlib 前提**のことが多い。**picolibc 系**ツールチェーンでは
  `nosys.specs`・`libstdc++`・一部 C++ 標準ヘッダ・`stdout`(FILE) の供給が異なり、リンク段で
  `stdout` 未定義・`-lc`/`specs` 不在等になる。対処は (a) SDK 想定の newlib 系ツールチェーンを使う
  （最も素直・推奨）、(b) picolibc 用に補完する（specs の差し替え・空 `libstdc++` スタブ・
  C++ ヘッダ shim・C ライブラリ選択の明示）。リンカスクリプト/`--gc-sections` を SDK 側と二重に
  与えない点にも注意。
- **分類**：移植/ツールチェーン（libc・リンカ）
- **プロセッサ**：非依存（ホスト側ツールチェーン依存。ARM/RISC-V いずれでも起こる）
- **出所リポ**：asp3_pico_sdk
- **実装非依存の根拠**：固有のパス/ファイル名/トリプルを本文に書かず、「newlib 系/picolibc 系
  ツールチェーン」「specs・libstdc++・stdout・CLIB 選択」で記述。

---

## 取り込み時の注記書式（例）

`〔分類: 制約/アーキ構造 ｜ プロセッサ: ARM Cortex-M / RISC-V(vectored mtvec) ｜ 出所: asp3_pico_sdk〕`
