---
name: asp3-pico-sdk-ops
description: >-
  本リポジトリ（asp3_pico_sdk＝TOPPERS/ASP3 を Raspberry Pi Pico SDK と協調動作させる SDK統合リポ）
  での「叩き方」固有skill。ASP3 を PICO2（RP2350）の ARM-S(Cortex-M33)／RISC-V(Hazard3) 向けに
  ビルド・openocd 書き込み・シリアル/GDB 観測するとき、VS Code Pico 拡張でターゲット(ARM↔RISC-V)を
  切り替えるとき、pico-sdk のバージョン(2.1.1/2.2.0)対応やツールチェーン(拡張同梱/システム)で
  詰まったとき、RISC-V の割込み(RX)が動かないとき、CMake 統合(asp3_pico_sdk.cmake・ASP3_TARGET・
  asp3_set_pico_sdk_options)を触るときに使う。ユーザが「pico-sdk」「PICO2」「RP2350」「Hazard3」
  「rp2350-arm-s」「rp2350-riscv」「pico に書き込み」「openocd」「シリアルが出ない」「ターゲット切替」
  「SDK 2.2.0」「ASP3 を pico で動かす」と言ったときに発動。
  境界：アプリ(タスク/セマフォ等)の C/.cfg の書き方は toppers-asp、カーネル/移植の共通規約は
  toppers-kernel-dev、症状→原因の診断概念は toppers-kernel-debug。本skillは「このリポでの具体コマンドと固有の落とし穴」に徹し、概念はそれらへ委譲する。
---

# asp3_pico_sdk 操作 skill（このリポでの叩き方）

TOPPERS/ASP3 を pico-sdk と協調させる **SDK統合リポ** の運用専用。規約・設計の正本は
リポジトリの `CLAUDE.md`／`docs/`／submodule `asp3/asp3_core/docs/dev/pico-sdk-integration.md`。
**概念は委譲**：アプリの書き方→`toppers-asp`／カーネル・移植規約→`toppers-kernel-dev`／
診断の切り分け→`toppers-kernel-debug`。本skillは具体コマンドと固有の落とし穴のみ。

## 0. 構成（要点だけ。詳細は CLAUDE.md §0）

- 純カーネルは submodule `asp3/asp3_core`。チップ/共通 arch も asp3_core を共用（SDK側 arch 重複は無し）。
- SDK固有は `asp3/target/{pico2_arm_sdk_gcc,pico2_riscv_sdk_gcc}` と協調ヘルパ `asp3/asp3_pico_sdk.cmake`。
- アプリは `sample1/`（ARM/RISC-V 共通・1フォルダ）。`timer_check/` 同梱。
- `PICO_PLATFORM` で ISA 切替（ソース・cfg は ISA 非依存）。

| PICO_PLATFORM | ASP3_TARGET（SDK） | openocd target | 拡張 toolchainVersion |
|---|---|---|---|
| `rp2350-arm-s` | `pico2_arm_sdk_gcc` | `target/rp2350.cfg` | `14_2_Rel1`（ARM, newlib） |
| `rp2350-riscv` | `pico2_riscv_sdk_gcc` | `target/rp2350-riscv.cfg` | `RISCV_RPI_2_0_0_5`（RISC-V, newlib） |

## 1. ビルド

**VS Code（推奨）**：`sample1/` を開き拡張の Compile/Run。ARM↔RISC-V は拡張の「Switch Board/Platform」。
拡張は `sample1/CMakeLists.txt` 冒頭の `set(toolchainVersion …)` を書き換え、`~/.pico-sdk/cmake/pico-vscode.cmake`
が RISC-V toolchain を検出して `PICO_PLATFORM=rp2350-riscv` と SDK パス(`~/.pico-sdk/sdk/<ver>`)を自動設定する。

**CLI**（ISA ごとに別ビルドディレクトリ。同一 build/ では ISA 切替不可＝キャッシュにツールチェーン固定）：
```bash
export PICO_SDK_PATH=/path/to/pico-sdk            # 2.1.1 / 2.2.0
cd sample1
cmake -S . -B build_arm   -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2 && cmake --build build_arm -j
cmake -S . -B build_riscv -DPICO_PLATFORM=rp2350-riscv -DPICO_BOARD=pico2 && cmake --build build_riscv -j
# 生成物: build_*/sample1_pico_sdk.uf2 / .elf
```
> CLI で RISC-V を拡張同梱 toolchain で再現するには `set(toolchainVersion RISCV_RPI_2_0_0_5)` に一時書換
> （`pico-vscode.cmake` が PICO_PLATFORM を自動設定）。検証目的なら戻すこと。

## 2. 書き込み（openocd / Debugprobe）

```bash
# ARM-S
openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg       -c "adapter speed 5000" -c "program build_arm/sample1_pico_sdk.elf verify reset exit"
# RISC-V
openocd -f interface/cmsis-dap.cfg -f target/rp2350-riscv.cfg -c "adapter speed 5000" -c "program build_riscv/sample1_pico_sdk.elf verify reset exit"
```
`Verified OK`／`Resetting Target` を確認。RISC-V 接続時は examine に `rp2350.rv0`(XLEN=32)、ARM は `rp2350.cm0` が出る。

## 3. シリアル観測（UART0・GP0(TX)/GP1(RX)・115200 8N1・USB stdio 無効）

```bash
stty -F /dev/ttyACM0 115200 raw -echo
cat /dev/ttyACM0 > /tmp/serial.log &        # 書き込み前に起動
# → バナー "TOPPERS/ASP3 Kernel ... <RISC-V Hazard3>/(ARM)" → "task1 is running (NNN)." の周期出力で基本動作OK
```
- ⚠️**落とし穴**：採取中に `: > /tmp/serial.log` で truncate すると、走っている `cat` が旧オフセットに書き続け
  **スパースファイル**になり grep/wc が誤検出する。採り直すときは `cat` を kill→**新ファイル**へ再起動する。
- `/dev/ttyACM0` は Debugprobe の UART ブリッジ（CMSIS-DAP は別 bulk）。書き込み(openocd)と同時に読める。

## 4. RISC-V の RX（受信割込み）テスト・GDB

`sample1` は `serial_rea_dat` でコマンド文字を受ける。RX が効いていれば応答が出る：
```bash
printf 'a'  > /dev/ttyACM0     # → "#act_tsk(1)"
printf '2'  > /dev/ttyACM0; printf 'a' > /dev/ttyACM0   # → "#act_tsk(2)"（複数文字RX）
```
GDB（riscv elf は `gdb-multiarch` が自動判別）：
```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2350-riscv.cfg -c "adapter speed 5000" &   # gdbサーバ :3333
gdb-multiarch build_riscv/sample1_pico_sdk.elf -ex "target extended-remote :3333" -ex "break sio_isr" -ex continue
```
> RISC-V の RX は **ASP3 ネイティブ ISR**（`target_serial.cfg` の `CRE_ISR(sio_isr)`）で受ける。pico-sdk の
> `irq_set_exclusive_handler` は使えない（mtvec 解釈が非互換）。**理由＝アーキ構造の制約**は概念なので
> `toppers-kernel-dev`（割込み共存）側の知見に委譲。ARM は `asp3_set_pico_sdk_options` が irq_* を `--wrap`。

## 5. pico-sdk バージョン / ツールチェーン

- **2.1.1 / 2.2.0 両対応**。2.2.0 は新設の `pico_platform_common`（`pico/platform/common.h`）が要るため、
  `asp3/target/*/target.cmake` の include に `src/rp2_common/pico_platform_common/include` を追加済み
  （2.1.1 では非存在 `-I` は無視＝無害）。他の統合依存（irq_*/exception_* の `--wrap` 8関数・
  `PICO_RUNTIME_*` 定義・stdout・boot_stage2 の `--specs=nosys.specs`）は 2.1.1→2.2.0 で不変。
- **RISC-V toolchain は拡張同梱（`RISCV_RPI_2_0_0_5`＝newlib）前提**。Ubuntu システムの
  `riscv64-unknown-elf-gcc` は **picolibc** ベースで、nosys.specs/libstdc++/C++ヘッダ等が無く回避策が要る
  （現在は撤去済。経緯は git 履歴と `asp3/asp3_core/docs/dev/pico-sdk-integration.md`）。

## 6. 2.2.0 等を CLI で検証する手順（拡張の SDK 上書きを回避）

```bash
git -C <pico-sdk> fetch --depth 1 origin tag 2.2.0 && git -C <pico-sdk> worktree add /tmp/pico-sdk-2.2.0 2.2.0
mv ~/.pico-sdk/cmake/pico-vscode.cmake ~/.pico-sdk/cmake/pico-vscode.cmake.bak   # env PICO_SDK_PATH を効かせる
export PICO_SDK_PATH=/tmp/pico-sdk-2.2.0 && cmake -S sample1 -B /tmp/b -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2 && cmake --build /tmp/b -j
mv ~/.pico-sdk/cmake/pico-vscode.cmake.bak ~/.pico-sdk/cmake/pico-vscode.cmake   # 必ず戻す
```

## 7. CMake 統合のポイント（新規アプリへの適用）

`asp3/asp3_pico_sdk.cmake` を `pico_sdk_init()` 後に include（sample1 からは `include(../asp3/asp3_pico_sdk.cmake)`）→ `PICO_PLATFORM` から `ASP3_TARGET`/
`ASP3_CORE_DIR` 解決 → `ASP3_LIBRARY_ONLY ON` で `add_subdirectory(${ASP3_CORE_DIR} asp3)` →
`asp3_add_syssvc(<t>)` → `target_link_libraries(<t> pico_stdlib asp3)` → `asp3_set_pico_sdk_options(<t>)`
（irq_* の `--wrap`・SDK ランタイム初期化抑止。ISA は自動分岐）。手順詳細は README §「新規プロジェクトへの適用方法」。

## 検証の鉄則（このリポ）
- 変更したら**必ずビルドを通してから報告**。実機確認はシリアル出力（`task1 is running`・周期）を根拠とする。
- 進捗・結果は `asp3/asp3_core/docs/dev/pico-sdk-integration.md` に追記（asp3_core は別リポ。push 権限が無ければ差分提示）。
