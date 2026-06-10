# CLAUDE.md — asp3_pico_sdk

TOPPERS/ASP3 Core を Raspberry Pi Pico SDK と協調動作させる **SDK統合リポジトリ**。
純カーネル（`asp3_core`）を submodule 参照し、pico 固有部だけを本リポジトリに持つ。

> 本ファイルは別マシン（**PICO2 実機が接続された PC**）で作業する Claude 向けの作業指示書。
> 設計の全体像・経緯は submodule 側 `asp3/asp3_core/docs/dev/pico-sdk-integration.md` が正本。

---

## 0. リポジトリ構成

```
asp3_pico_sdk/
├── asp3/                            ← ASP3移植部を集約（カーネル submodule＋チップarch＋SDK target）
│   ├── asp3_core/                   ← submodule（純カーネル：kernel/ cfg/ syssvc/ 共通arch）※public
│   ├── arch/arm_m_gcc/rp2350/       ← チップ依存部（ARM-S・SDK版＝pico-sdk serial 使用）
│   │                                   ※tool依存部(arch/gcc)は asp3_core 側を共用（重複コピー廃止）
│   └── target/
│       ├── pico2_arm_sdk_gcc/       ← ターゲット依存部（ARM-S・ASP3_TARGET_DIR で供給）
│       └── pico2_riscv_sdk_gcc/     ← ターゲット依存部（RISC-V・タスク2/作業中）
├── asp3_pico_sdk.cmake              ← 協調ヘルパ（PICO_PLATFORM→ASP3_TARGET/ASP3_TARGET_DIR・irq_* の --wrap）
└── sample1/                         ← アプリ（CMakeLists.txt・pico_sdk_import.cmake・sample1_pico_sdk.c）
```

- ビルドは `asp3_core` の正準 CMakeLists を **ライブラリ専用モード**（`ASP3_LIBRARY_ONLY=ON`）で
  `add_subdirectory` し、最終 ELF は本リポジトリ側でリンクする（fork CMake は廃止済み）。
- 共存の要：pico-sdk の割込み登録API（`irq_*`/`exception_*` 8関数）を **リンカ `--wrap` で
  ASP3 管理へ誘導**＋`PICO_RUNTIME_SKIP_INIT_*` で SDK のベクタ/IRQ優先度初期化を抑止
  （ASP3 が NVIC を掌握）。実装は `asp3_pico_sdk.cmake` の `asp3_set_pico_sdk_options()`。

## 1. ⚠️ 禁則（作業前に必読）

1. **`asp3/asp3_core/`（submodule）配下を直接編集しない**。カーネル本体は上流 ASP3 追従領域。
   変更が必要なら asp3_core リポジトリ側で行い、その `AGENTS.md` の規約（`kernel/`・`include/`・
   `library/` 編集禁止、変更は `target/`・`syssvc/`・新規ファイルに限定）に従う。
   本リポジトリでの作業は **SDK 側ファイル（`asp3/arch/`・`asp3/target/`・`asp3_pico_sdk.cmake`・`sample1/`）**
   に閉じるのが原則。
2. **カーネル内で動的メモリ確保を使わない**（`malloc`/`new` 等禁止。静的生成のみ。ASP3 安全設計方針）。

## 2. 取得・ビルド・実機確認（基本フロー）

```bash
# 取得（asp3_core は public なので匿名で submodule 取得可）
git clone --recurse-submodules https://github.com/exshonda/asp3_pico_sdk.git
cd asp3_pico_sdk
# 既存clone: git submodule update --init --recursive

# ビルド（ARM-S）
export PICO_SDK_PATH=/path/to/pico-sdk          # 2.1.1
cd sample1
cmake -S . -B build -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2
cmake --build build -j
# 生成物: build/sample1_pico_sdk.uf2 / .elf / .bin
```

書き込み：BOOTSEL を押しながら USB 接続 →`RPI-RP2` に `.uf2` をコピー（または
`picotool load -x build/sample1_pico_sdk.uf2`、SWD は
`openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "program build/sample1_pico_sdk.elf verify reset exit"`）。

シリアル確認：**UART0・GP0(TX)/GP1(RX)・115200 8N1**（USB stdio は無効）。
`minicom -D /dev/ttyUSB0 -b 115200` 等で
`TOPPERS/ASP3 Kernel ...` → `task1 is running (NNN).` の周期出力が出れば基本動作OK。

> 前提ツール：pico-sdk 2.1.1 / arm-none-eabi-gcc（14_2_Rel1 推奨）/ cmake≥3.13 / python3 / picotool。
> 既知の検証実績（ARM-S）：`.uf2` 生成・cfg 3パス・`--wrap` リンク・**PICO2 実機で task1 出力**を確認済
> （`asp3/asp3_core/docs/dev/pico-sdk-integration.md` 実施結果）。

## 検証の鉄則

- コードを変更したら **必ずビルドを通してから報告**。「動くはず」で報告しない。
- 実機確認はシリアル出力（`task1 is running`・周期）を根拠とする。
- 変更後、進捗・結果を `asp3/asp3_core/docs/dev/pico-sdk-integration.md` の「未完（後続）」に追記する
  （asp3_core は別リポジトリ。push 権限が無ければ差分を提示してユーザーに push を依頼）。

---

# タスク

## タスク1：タイマ競合の検証と調停（SDK alarm TIMER0 vs ASP3 HRT）

### 背景
- ASP3 は `USE_TIM_AS_HRT` で**高分解能タイマ（HRT）に TIMER0 を使用**する
  （`asp3/target/pico2_arm_sdk_gcc/target_timer.c` ／ `target_kernel.cfg`）。
- 一方 pico-sdk の `hardware_timer`/`hardware_alarm`（`busy_wait`・`add_alarm_*`・`sleep_ms` 等）も
  **TIMER0 のアラームを使う**。両者が同じ TIMER0 を取り合う可能性がある。
- 基本サンプル（sample1）は実機で動作したが、**時間系の定量検証は未実施**＝本タスク。

### やること
1. **現状把握**：`target_timer.c` が TIMER0 のどのアラーム/比較器・割込みを使うか、
   pico-sdk 既定のアラームプール（`PICO_TIME_DEFAULT_ALARM_POOL_*`）と衝突するかを特定する。
   `asp3/asp3_core/arch/arm_m_gcc/common/core_timer.c`（HRT 共通ロジック）も参照。
2. **定量検証（実機）**：sample1 で
   - `dly_tsk(N)`／周期ハンドラ（`CRE_CYC`）の周期が設計どおりか（例：1秒周期が実測ほぼ1秒か）
   - `get_tim()`／`fch_hrt()` が単調増加し、長時間（数分〜）でドリフト・ロールオーバ異常が無いか
   を測定（オシロ/ロジアナ or GPIO トグル＋シリアルのタイムスタンプ）。
3. **共存設計**：アプリが pico-sdk のタイマAPI（`sleep_ms`/`add_alarm_in` 等）も使いたい場合の方針を決める。
   候補：(a) ASP3 HRT を別タイマ/比較器に逃がす、(b) pico-sdk alarm pool を別チャネルに固定、
   (c) SDK タイマAPIは使用禁止としドキュメント明記。**カーネル側変更が要るなら asp3_core 側で**
   （禁則①）。
4. 結果（測定値・採用方針）を `asp3/asp3_core/docs/dev/pico-sdk-integration.md` に記録。

### 着手の入口
- `asp3/target/pico2_arm_sdk_gcc/target_timer.{c,h}`、`target_kernel.cfg`、`asp3/asp3_core/arch/arm_m_gcc/common/core_timer.c`
- pico-sdk：`src/rp2_common/hardware_timer/`、`src/common/pico_time/`

## タスク2：RISC-V（Hazard3）版の追加（PICO_PLATFORM=rp2350-riscv）

### 背景
- 現在 `asp3_pico_sdk.cmake` は `PICO_PLATFORM=rp2350-riscv` を `FATAL_ERROR`（未対応）にしている。
- 移植元は **asp3_core のベアメタル RISC-V 実装**：
  - `asp3/asp3_core/arch/riscv_gcc/rp2350/`（チップ：`chip.cmake`・`xh3irq_kernel_impl.h`＝**Xh3irq 割込み**・`chip_support.S` 等）
  - `asp3/asp3_core/arch/riscv_gcc/common/`（コア：`core_*`・`start.S`・`mtimer.*`・`plic_*`・`riscv.h`。XLEN抽象 RV32/RV64）
  - `asp3/asp3_core/target/pico2_riscv_gcc/`（`USE_TIM_AS_HRT`・`chip.cmake` include）
- RP2350 の RISC-V コア（Hazard3）は割込みが **NVIC ではなく Xh3irq**、ブートも RISC-V IMAGE_DEF。
  ARM-S 版とは arch/target が根本的に異なる（ISAごとに別物）。

### やること（ARM-S 構成を雛形に）
1. **ヘルパ拡張**：`asp3_pico_sdk.cmake` の `rp2350-riscv` 分岐を実装
   （`set(ASP3_TARGET pico2_riscv_sdk_gcc)` ＋ `ASP3_TARGET_DIR` 解決）。
2. **チップ依存部を SDK 向けに用意**：`asp3/arch/riscv_gcc/rp2350/` を本リポジトリに追加。
   asp3_core のベアメタル版をベースに、**シリアルを pico-sdk stdio 経由へ**（`TOPPERS_OMIT_CHIP_SERIAL`）、
   CMake はパス規約（チップ＝`CMAKE_CURRENT_LIST_DIR` 相対、共通 arch＝`ASP3_ROOT_DIR`）に合わせる
   （ARM-S の `asp3/arch/arm_m_gcc/rp2350/arch.cmake`・`asp3/target/pico2_arm_sdk_gcc/target.cmake` が参考）。
3. **ターゲット依存部**：`asp3/target/pico2_riscv_sdk_gcc/` を追加
   （`target_kernel.cfg`・`target_kernel.py`・`target_check.py`・`target_timer.c`・`target_serial.c` 等）。
4. **割込み共存の確認**：pico-sdk の RISC-V ビルドが `irq_*` をどう登録するか調べ、ARM 版の `--wrap`
   8関数が RISC-V でも適用可能か／別関数かを確認（Hazard3 の外部割込みコントローラ＝Xh3irq 経由）。
   必要なら `asp3_set_pico_sdk_options()` を ISA で分岐。
5. **sample1 は ISA で分けない**：`PICO_PLATFORM` で arm/riscv を切替（ソースは ISA 非依存）。
6. **ビルド検証**：`cmake -S . -B build_riscv -DPICO_PLATFORM=rp2350-riscv -DPICO_BOARD=pico2`。
   RISC-V トロイチェーン（pico-sdk の RISC-V toolchain、例 `riscv-none-elf-gcc`）が必要。
   実機（PICO2 の RISC-V コア）で task1 出力まで確認。
7. 結果を `asp3/asp3_core/docs/dev/pico-sdk-integration.md` に記録。

### 着手の入口
- `asp3_pico_sdk.cmake`（分岐追加）
- 移植元：`asp3/asp3_core/arch/riscv_gcc/{rp2350,common}/`、`asp3/asp3_core/target/pico2_riscv_gcc/`
- asp3_core 側の経緯：`asp3/asp3_core/docs/dev/pico2-riscv.md`（Xh3irq・RV32分岐・dlynse較正等）

---

## 参考リポジトリ

| リポジトリ | 用途 |
|---|---|
| `asp3_core`（submodule・public） | 純カーネル。`docs/dev/pico-sdk-integration.md`＝統合の正本。`docs/porting/PORTING_GUIDE.md`「外部ターゲット」節＝パス規約 |
| `asp3_pico_sdk_legacy`（archived） | 旧・カーネル同梱fork。移植元の参照点（読取専用） |
