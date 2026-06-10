# TOPPERS/ASP3 Core + Raspberry Pi Pico SDK sample (sample1)
#
#  旧 asp3_pico_sdk の fork CMake は廃止し，asp3_core の正準 CMakeLists を
#  ライブラリ専用モード（ASP3_LIBRARY_ONLY）で add_subdirectory して使う．
#  pico 差分は ../asp3_pico_sdk.cmake ヘルパ＋外部 target（ASP3_TARGET_DIR）に閉じる．
#  経緯：asp3_core/docs/dev/pico-sdk-integration.md

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# ISA：本ディレクトリは ARM-S（Cortex-M33）専用．既定を設定（-D で上書き可）．
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Pico platform (ISA)")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(sample1_pico_sdk C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# TOPPERS/ASP3 Core ＋ Pico SDK 協調ヘルパ
#  （PICO_PLATFORM→ASP3_TARGET/ASP3_TARGET_DIR・ASP3_CORE_DIR・--wrap設定）
include(../asp3_pico_sdk.cmake)

# アプリ（タスク定義＝sample1.c/.cfg）は asp3_core 同梱の sample を使用．
# main() は pico ラッパ（sample1_pico_sdk.c）が提供する．
set(ASP3_APPLDIR ${ASP3_CORE_DIR}/sample)
set(ASP3_APPLNAME sample1)

# asp3 カーネルライブラリ（libasp3.a）をライブラリ専用モードでビルド．
# asp 実行ファイル・サンプル・ctest は作らず，asp3 lib／cfg生成／
# ヘルパ関数（asp3_add_syssvc 等）のみが公開される．
set(ASP3_LIBRARY_ONLY ON CACHE BOOL "build asp3 as library only" FORCE)
add_subdirectory(${ASP3_CORE_DIR} asp3)

# 最終実行ファイル（タスク本体＋pico main ラッパ）
add_executable(sample1_pico_sdk
    ${ASP3_CORE_DIR}/sample/sample1.c
    sample1_pico_sdk.c
)

# システムサービス（syslog/banner/serial/serial_cfg/logtask＋library）を追加．
# 旧 fork の syssvc.cmake＋library.cmake に相当する asp3_core のヘルパ．
asp3_add_syssvc(sample1_pico_sdk)

pico_set_program_name(sample1_pico_sdk "sample1_pico_sdk")
pico_set_program_version(sample1_pico_sdk "0.1")

# 標準入出力：UART有効・USB無効（変更可）
pico_enable_stdio_uart(sample1_pico_sdk 1)
pico_enable_stdio_usb(sample1_pico_sdk 0)

target_include_directories(sample1_pico_sdk PRIVATE
    ${ASP3_CORE_DIR}/sample
    ${CMAKE_CURRENT_LIST_DIR}
)

# pico 標準ライブラリ ＋ asp3 カーネルライブラリ
target_link_libraries(sample1_pico_sdk
    pico_stdlib
    asp3
)

# TOPPERS/ASP3 を使うための Raspberry Pi Pico SDK の設定（irq_* の --wrap 等）
asp3_set_pico_sdk_options(sample1_pico_sdk)

pico_add_extra_outputs(sample1_pico_sdk)
