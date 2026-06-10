#
#  外部（SDK）ターゲットのパス解決規約（asp3_core PORTING_GUIDE「外部ターゲット」）：
#   - 共通arch（arch/riscv_gcc/rp2350, arch/riscv_gcc/common）は asp3_core
#     サブモジュール側（ASP3_ROOT_DIR）を参照
#   - ターゲット依存部は本リポジトリ側（CMAKE_CURRENT_LIST_DIR）
#     （本ファイルは asp3_core の CMakeLists から ASP3_TARGET_DIR 経由で
#      include されるため，CMAKE_CURRENT_LIST_DIR は本ターゲットディレクトリを指す）
#

set(TARGETDIR ${CMAKE_CURRENT_LIST_DIR})

list(APPEND ASP3_CFG_FILES
    ${TARGETDIR}/target_kernel.cfg
)

list(APPEND ASP3_KERNEL_CFG_TRB_FILES
    ${TARGETDIR}/target_kernel.py
)

list(APPEND ASP3_CHECK_TRB_FILES
    ${TARGETDIR}/target_check.py
)

#
#  pico-sdk のインクルードパス（ARM-S版と同じ．RISC-V固有のものはchip.cmakeが追加）
#
list(APPEND ASP3_INCLUDE_DIRS
    ${CMAKE_BINARY_DIR}/generated/pico_base
    ${PICO_SDK_PATH}/src/common/pico_base_headers/include
    ${PICO_SDK_PATH}/src/common/hardware_claim/include
    ${PICO_SDK_PATH}/src/common/pico_sync/include
    ${PICO_SDK_PATH}/src/common/pico_time/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_runtime_init/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_runtime/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_stdio/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_base/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_exception/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_irq/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_sync/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_sync_spin_lock/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_timer/include
    ${PICO_SDK_PATH}/src/rp2350/pico_platform/include
    ${PICO_SDK_PATH}/src/rp2350/hardware_regs/include
    ${PICO_SDK_PATH}/src/rp2350/hardware_structs/include
    ${TARGETDIR}
)

list(APPEND ASP3_COMPILE_DEFS
    PICO_RP2350
    USE_TIM_AS_HRT
    # Hazard3はFPUなし（TOPPERS_FPU_*は不要）
)

#  picolibc ヘッダ（stdio.h 等）のインクルードパス
#  riscv64-unknown-elf-gcc (Ubuntu) は picolibc ベースで，
#  --specs=picolibc.specs なしではデフォルト検索パスに含まれないため明示する
list(APPEND ASP3_INCLUDE_DIRS
    /usr/lib/picolibc/riscv64-unknown-elf/include
)

list(APPEND ASP3_TARGET_C_FILES
    ${TARGETDIR}/target_kernel_impl.c
    ${TARGETDIR}/target_timer.c
    ${TARGETDIR}/target_serial.c
)

#
#  cfg1_out（静的APIの値抽出用）のリンク設定
#  ARM-S SDK版と同様：-nostdlib -nostartfiles でシンボル抽出専用ELFをリンク
#  （cfg1_out は実行しないため start.S・libc スタートアップは不要）
#
list(APPEND ASP3_LINK_OPTIONS
    -nostdlib
    -nostartfiles
)
list(APPEND ASP3_LINK_LIBS gcc)

#
#  chip.cmake のインクルード（Xh3irq・RV32IMAコンパイラフラグ・chip_kernel_impl.c等）
#  chip.cmake は ASP3_LINK_LIBS・ASP3_START_FILES を操作するため最後にインクルードする
#
include(${ASP3_ROOT_DIR}/arch/riscv_gcc/rp2350/chip.cmake)

#
#  SDK向け修正：chip.cmake のベアメタル向け設定を上書き
#

#  start.S: pico-sdk が独自スタートアップを提供するため不要
#  （cfg1_out は実行しないのでエントリポイントも不要．ARM-S と同じ扱い）
list(REMOVE_ITEM ASP3_START_FILES
    ${ASP3_ROOT_DIR}/arch/riscv_gcc/common/start.S
)

#  chip.cmake の -march/-mabi は asp3 ライブラリの PUBLIC オプションとして
#  pico-sdk のアプリ（sample1_pico_sdk）にも伝播する．
#  pico-sdk は CMAKE_C_FLAGS に rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb を
#  設定しているため，chip.cmake の縮退版（_zicsr_zifencei のみ）を除去する．
#  （pico-sdk の arch フラグが優先され Zbs 等が有効になる）
list(REMOVE_ITEM ASP3_COMPILE_OPTIONS
    -march=rv32imac_zicsr_zifencei
    -mabi=ilp32
)

#  libc_stub.c は不要（picolibc を使用するため）
list(REMOVE_ITEM ASP3_ARCH_C_FILES
    ${ASP3_ROOT_DIR}/arch/riscv_gcc/polarfire_soc/libc_stub.c
)

#  ベアメタル用シリアルドライバ → target_serial.c（pico-sdk stdio）に置き換え
list(REMOVE_ITEM ASP3_SYSSVC_TARGET_C_FILES
    ${RP2350_ARM_CHIPDIR}/chip_serial.c
    ${RP2350_ARM_CHIPDIR}/rp2350_uart.c
)
