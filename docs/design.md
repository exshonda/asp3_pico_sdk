# SDK協調の設計

TOPPERS/ASP3 Core（submodule）と pico-sdk を**同一バイナリで共存**させるための
設計判断の記録。実装の入口は `asp3/asp3_pico_sdk.cmake`（協調ヘルパ）。

## 1. 全体構成（ライブラリ専用モード）

- asp3_core を `add_subdirectory(${ASP3_CORE_DIR} asp3)` ＋ **`ASP3_LIBRARY_ONLY=ON`** で取り込む。
  asp 実行ファイル・サンプル・ctest は作られず、`asp3` ライブラリ・cfg 3パス生成・
  ヘルパ関数（`asp3_add_syssvc` 等）だけが公開される。最終 ELF は pico-sdk 側
  （`pico_stdlib` をリンクする `add_executable`）が作る。
- ターゲット依存部は本リポジトリの `asp3/target/pico2_{arm,riscv}_sdk_gcc/` を
  **`ASP3_TARGET_DIR`** で供給。チップ依存部（arch）は **asp3_core 側を共用**
  （`${ASP3_ROOT_DIR}/arch/{arm_m_gcc,riscv_gcc}/rp2350/chip.cmake` を直接 include。
  SDK 側 arch の重複は全廃済み＝SDK 固有なのは target 依存部のみ）。
- target.cmake から **pico-sdk 提供物を除外**する点が SDK 統合の本質：
  スタートアップ（start.S）・チップ内蔵シリアル（chip_serial.c/rp2350_uart.c）は
  `list(REMOVE_ITEM ...)` で外し、pico-sdk の crt0／stdio を使う
  （`TOPPERS_OMIT_CHIP_SERIAL`・`TOPPERS_OMIT_VECTOR_TABLE`・`OMIT_ISTACK`）。

## 2. 割込み共存（ISA で方式が根本的に異なる）

### ARM-S（Cortex-M33・NVIC）

ASP3 が NVIC・ベクタテーブルを掌握する。pico-sdk 側の登録 API を**リンカ `--wrap` で
ASP3 管理へ誘導**（8関数：`exception_{get_vtable_handler,set_exclusive_handler,
restore_handler}`・`irq_{get_vtable_handler,set_exclusive_handler,add_shared_handler,
remove_handler,add_tail_to_free_list}`）。`__wrap_*` の実装は
`asp3/target/pico2_arm_sdk_gcc/target_kernel_impl.c`（ASP3 の `_kernel_exc_tbl` へ登録）。

あわせて SDK のベクタ/IRQ 優先度初期化を抑止：
`PICO_RUNTIME_NO_INIT_INSTALL_RAM_VECTOR_TABLE`・
`PICO_RUNTIME_SKIP_INIT_INSTALL_RAM_VECTOR_TABLE`・
`PICO_RUNTIME_SKIP_INIT_PER_CORE_IRQ_PRIORITIES`。

### RISC-V（Hazard3・Xh3irq）— `--wrap` は成立しない

**mtvec の解釈が非互換**：pico-sdk は mtvec を「IRQ 番号で索引する関数ポインタ表」と
みなすが、ASP3 は mtvec を VECTORED モードの **cause 索引 jump 表**に使い、外部割込みは
`meinext`（Xh3irq）でデマルチプレクスして **const の `_kernel_inh_table`** へ振る。
このため pico-sdk の `irq_set_exclusive_handler()` を受けると hard_assert で PANIC する
（ARM の「RAM ベクタ表へ `--wrap` で流し込む」トリックが構造的に使えない）。

採用した**ハイブリッド方式**（SDK 側のみで完結・asp3_core 変更なし）：
- **RX は ASP3 ネイティブ ISR**：`target_serial.cfg` に `CFG_INT(USART_INTNO)`＋
  `CRE_ISR(SIO_RX_ISR, sio_isr)` を静的登録。`sio_isr` が `uart0_hw->dr` を直接読み
  RXIC/RTIC をクリアする。pico-sdk の irq 登録 API は一切呼ばない。
- **TX は pico-sdk stdio**（`putchar_raw`）。
- `_kernel_inh_table` の RAM 化は不要だった。
- SDK 初期化の抑止は `PICO_RUNTIME_SKIP_INIT_PER_CORE_IRQ_PRIORITIES` のみ
  （mtvec は ASP3 の `chip_initialize()` が `trap_vector_table` へ向けて掌握）。

> アプリへの含意：**RISC-V では pico-sdk の `irq_*` 登録 API を使わないこと**。
> 割込みを使う場合は ASP3 の静的 API（`CRE_ISR`/`DEF_INH`＋`CFG_INT`）で登録する。
> ARM-S では `--wrap` 経由で pico-sdk API がそのまま使える。

## 3. ツールチェーン

| ISA | ツールチェーン | 備考 |
|---|---|---|
| ARM-S | VS Code Pico 拡張同梱 `14_2_Rel1`（システム arm-none-eabi-gcc 13.x でも可） | |
| RISC-V | **拡張同梱の RISC-V toolchain（`RISCV_RPI_*`＝riscv32-unknown-elf／newlib）前提** | Ubuntu システムの riscv64/picolibc ツールチェーンで動かすための回避策（`-Tpicolibc.ld` 除去 specs・libstdc++ スタブ等）は一時導入したが**全廃済み**。newlib 前提に一本化 |

VS Code 拡張の ISA 切替（Switch Platform）は `sample1/CMakeLists.txt` 冒頭ブロックの
`toolchainVersion` を書換え、`pico-vscode.cmake` がそこから `PICO_PLATFORM` を導出する。
CLI では `-DPICO_PLATFORM=` を渡し **ISA ごとに別ビルドディレクトリ**を使う
（CMake キャッシュにツールチェーンが固定されるため同一ディレクトリで切替不可）。

## 4. pico-sdk バージョン互換（2.1.1 / 2.2.0）

`target.cmake` は asp3 ライブラリのビルド用に **pico-sdk の include ディレクトリを
手動列挙**している（pico-sdk の INTERFACE ターゲットに依存しないため）。この方式の
保守点として、SDK のヘッダ再配置に追従が要る：

- **2.2.0**：`pico_platform_common`（`pico/platform/common.h`）が分離された。
  ARM/RISC-V 両 target.cmake に `src/rp2_common/pico_platform_common/include` を追加して対応
  （2.1.1 では存在しない `-I` は無視されるだけ＝両対応）。
- それ以外の統合依存点（`--wrap` 8関数のシグネチャ・`PICO_RUNTIME_*` 定義・
  stdio・boot_stage2）は 2.1.1→2.2.0 で安定。

新しい pico-sdk への追従時は、まず include 列挙と `--wrap` 対象関数の存在を確認すること。

## 5. パス規約（外部ターゲット）

asp3_core PORTING_GUIDE「外部（SDK）ターゲットの置き方」に従う：
- 共通 arch・カーネル＝`${ASP3_ROOT_DIR}`（submodule）
- ターゲット依存部＝`${CMAKE_CURRENT_LIST_DIR}` 相対（本リポジトリ）
- 協調ヘルパは `asp3/asp3_pico_sdk.cmake`（**asp3_fsp / stm32_vscode_asp と同配置**。
  `ASP3_CORE_DIR`＝`asp3/asp3_core`、`ASP3_TARGET_DIR`＝`asp3/target/<target>` を解決）
