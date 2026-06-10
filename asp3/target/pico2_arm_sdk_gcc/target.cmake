#
#  外部（SDK）ターゲットのパス解決規約（asp3_core PORTING_GUIDE「外部ターゲット」）：
#   - 共通arch（arch/arm_m_gcc/common）＋チップ依存部（arch/arm_m_gcc/rp2350）は
#     asp3_core サブモジュール側＝ASP3_ROOT_DIR を共用する（RISC-V版と同方針）．
#     チップ arch の SDK 側重複コピーは廃止（chip.cmake を直接 include）．
#   - ターゲット依存部は本リポジトリ側＝CMAKE_CURRENT_LIST_DIR 相対
#     （本ファイルは asp3_core の CMakeLists から ASP3_TARGET_DIR 経由で include される
#      ため，CMAKE_CURRENT_LIST_DIR は本ターゲットディレクトリを指す）
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
    #  コア種別の定義（旧 SDK arch.cmake から target.cmake 側へ移設．
    #  asp3_core の chip.cmake/common/arch.cmake はこれらを積まない規約のため）
    TOPPERS_CORTEX_M33
    TOPPERS_ENABLE_TRUSTZONE
    __TARGET_ARCH_THUMB=5
    __TARGET_FPU_FPV4_SP
    TBITW_IPRI=4
    TOPPERS_FPU_ENABLE
    TOPPERS_FPU_LAZYSTACKING
    TOPPERS_FPU_CONTEXT
)

list(APPEND ASP3_TARGET_C_FILES
    ${TARGETDIR}/target_kernel_impl.c
    ${TARGETDIR}/target_timer.c
    ${TARGETDIR}/target_serial.c
)

#
#  cfg1_out（静的APIの値抽出用の使い捨てELF）のリンク設定．
#
#  ライブラリ専用モード（ASP3_LIBRARY_ONLY=ON）では asp 実行ファイルは作らず，
#  ASP3_LINK_OPTIONS / ASP3_LINK_LIBS は asp3_core 側の cfg1_out のみが使う．
#  cfg1_out は main を持たないため -nostdlib -nostartfiles で素リンクする
#  （最終実行ファイルは本リポジトリ側が pico-sdk のスタートアップ・リンカ
#   スクリプトでビルドするため，これらは最終exeには波及しない）．
#  アーキフラグ（-mcpu 等）は pico-sdk が設定する CMAKE_C_FLAGS から得る．
#
list(APPEND ASP3_LINK_OPTIONS
    -nostdlib
    -nostartfiles
)
list(APPEND ASP3_LINK_LIBS c gcc nosys)

#
#  チップ依存部（asp3_core 側）を直接 include する（RISC-V 版と同方針）．
#  chip.cmake は common/arch.cmake を取り込み，core_*・chip_* と CHIPDIR インクルードを供給する．
#
include(${ASP3_ROOT_DIR}/arch/arm_m_gcc/rp2350/chip.cmake)

#
#  SDK向け修正：chip.cmake／common/arch.cmake のベアメタル向け設定を上書き
#
#  start.S は不要（pico-sdk が独自スタートアップを提供．cfg1_out は実行しない）
list(REMOVE_ITEM ASP3_ARCH_C_FILES
    ${ASP3_ROOT_DIR}/arch/arm_m_gcc/common/start.S
)
#  ベアメタル用シリアル（chip_serial.c・rp2350_uart.c）→ target_serial.c（pico-sdk stdio）に置換
list(REMOVE_ITEM ASP3_SYSSVC_TARGET_C_FILES
    ${ASP3_ROOT_DIR}/arch/arm_m_gcc/rp2350/chip_serial.c
    ${ASP3_ROOT_DIR}/arch/arm_m_gcc/rp2350/rp2350_uart.c
)
