#
#  外部（SDK）ターゲットのパス解決規約（asp3_core PORTING_GUIDE「外部ターゲット」）：
#   - 共通arch（arch/arm_m_gcc/common）は asp3_core サブモジュール側＝ASP3_ROOT_DIR
#   - チップ依存部・ターゲット依存部は本リポジトリ側＝CMAKE_CURRENT_LIST_DIR 相対
#     （本ファイルは asp3_core の CMakeLists から ASP3_TARGET_DIR 経由で include される
#      ため，CMAKE_CURRENT_LIST_DIR は本ターゲットディレクトリを指す）
#
set(ARCHDIR ${ASP3_ROOT_DIR}/arch/arm_m_gcc)
get_filename_component(CHIPDIR ${CMAKE_CURRENT_LIST_DIR}/../../arch/arm_m_gcc/rp2350 ABSOLUTE)
get_filename_component(GCCDIR  ${CMAKE_CURRENT_LIST_DIR}/../../arch/gcc ABSOLUTE)
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

include(${CHIPDIR}/arch.cmake)
