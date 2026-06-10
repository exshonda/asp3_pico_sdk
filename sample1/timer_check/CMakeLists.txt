# タイマ精度定量検証プログラム（TIMER0 ALARM0 vs SDK alarm pool 競合評価）
#
# ビルド:
#   cd sample1/timer_check
#   cmake -S . -B build -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2
#   cmake --build build -j
# 書き込み:
#   openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg \
#     -c "adapter speed 5000" \
#     -c "program build/timer_check_pico_sdk.elf verify reset exit"

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico2 CACHE STRING "Board type")

# pico_sdk_import.cmake は親ディレクトリ（sample1/）から参照
include(${CMAKE_CURRENT_SOURCE_DIR}/../pico_sdk_import.cmake)

project(timer_check_pico_sdk C CXX ASM)

pico_sdk_init()

# TOPPERS/ASP3 Core + Pico SDK 協調ヘルパ
include(${CMAKE_CURRENT_SOURCE_DIR}/../../asp3_pico_sdk.cmake)

# アプリディレクトリ = 本ディレクトリ（timer_check.cfg が置かれている場所）
set(ASP3_APPLDIR ${CMAKE_CURRENT_LIST_DIR})
set(ASP3_APPLNAME timer_check)

# asp3 カーネルライブラリをライブラリ専用モードでビルド
set(ASP3_LIBRARY_ONLY ON CACHE BOOL "build asp3 as library only" FORCE)
add_subdirectory(${ASP3_CORE_DIR} asp3)

# 実行ファイル（タイマ検証タスク + pico main ラッパ）
add_executable(timer_check_pico_sdk
    timer_check.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../sample1_pico_sdk.c
)

# システムサービス（syslog/banner/serial/logtask + library）
asp3_add_syssvc(timer_check_pico_sdk)

pico_set_program_name(timer_check_pico_sdk "timer_check_pico_sdk")
pico_set_program_version(timer_check_pico_sdk "0.1")

# UART 有効・USB stdio 無効
pico_enable_stdio_uart(timer_check_pico_sdk 1)
pico_enable_stdio_usb(timer_check_pico_sdk 0)

target_include_directories(timer_check_pico_sdk PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(timer_check_pico_sdk
    pico_stdlib
    asp3
)

# pico-sdk 割込みAPI を ASP3 管理へ誘導（--wrap 等）
asp3_set_pico_sdk_options(timer_check_pico_sdk)

pico_add_extra_outputs(timer_check_pico_sdk)
