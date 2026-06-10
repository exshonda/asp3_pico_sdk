#
#  TOPPERS/ASP3 Core ＋ Raspberry Pi Pico SDK 協調動作ヘルパ
#
#  sample1/CMakeLists.txt から project()・pico_sdk_init() の後に include する．
#  役割：
#   - PICO_PLATFORM → ASP3_TARGET / ASP3_TARGET_DIR の解決
#   - SDK協調に必要なカーネル構成定義（OMIT_ISTACK 等）の供給
#   - asp3_set_pico_sdk_options()：pico-sdk の割込み登録API（irq_*/exception_*）を
#     リンカ --wrap で ASP3 管理へ誘導＋SDKのベクタ/IRQ優先度初期化を抑止
#
#  カーネル本体・共通arch・cfgエンジンは asp3_core サブモジュール（ASP3_CORE_DIR）
#  側に置き，チップ依存部（asp3/arch/arm_m_gcc/rp2350）とターゲット依存部
#  （asp3/target/pico2_arm_sdk_gcc）は本リポジトリ側に置く（ASP3_TARGET_DIR で供給）．
#  asp3_core 側の受け入れ口は docs/dev/pico-sdk-integration.md を参照．
#

#  本リポジトリ（SDK側）のルート＝このファイルの場所
set(ASP3_SDK_DIR ${CMAKE_CURRENT_LIST_DIR})

#  asp3_core サブモジュール（純カーネル：kernel/ cfg/ syssvc/ 共通arch）
#  ASP3移植部（asp3_core submodule・チップ依存 arch・SDK target）は asp3/ 配下に集約．
#  ヘルパ（本ファイル）とアプリ（sample1）はリポジトリルートに置く．
set(ASP3_CORE_DIR ${CMAKE_CURRENT_LIST_DIR}/asp3/asp3_core)

#  ASP3カーネルソースのルート＝asp3_core サブモジュール．
#  asp3_add_syssvc() 等のヘルパ関数は呼び出し側スコープの ASP3_ROOT_DIR を参照する
#  （add_subdirectory(asp3_core) の子スコープで設定される値は親へ伝播しないため，
#   親スコープ＝本ファイルを include する sample 側でも明示的に設定しておく）．
set(ASP3_ROOT_DIR ${ASP3_CORE_DIR})

#  pico-sdk のチップ内蔵シリアル（chip_serial.c）は使わず，stdio（pico_stdlib）経由
set(TOPPERS_OMIT_CHIP_SERIAL true)

#  PICO_PLATFORM → ASP3_TARGET の解決
if (PICO_PLATFORM STREQUAL "rp2350-arm-s")
    set(ASP3_TARGET pico2_arm_sdk_gcc)
elseif (PICO_PLATFORM STREQUAL "rp2040")
    message(FATAL_ERROR "not supported ${PICO_PLATFORM} (ASP3 is single-core; use FMP3 for rp2040)")
elseif (PICO_PLATFORM STREQUAL "rp2350-riscv")
    set(ASP3_TARGET pico2_riscv_sdk_gcc)
else()
    message(FATAL_ERROR "not supported ${PICO_PLATFORM}")
endif()

#  ターゲット依存部（target.cmake）の場所を asp3_core へ供給（本リポジトリ側）．
#  asp3_core.cmake はこれを未定義時のみ既定値で埋めるため，ここで先に設定する．
set(ASP3_TARGET_DIR ${CMAKE_CURRENT_LIST_DIR}/asp3/target/${ASP3_TARGET})

#  SDK協調に必要なカーネル構成定義（add_subdirectory(asp3_core) の子スコープへ継承される）
list(APPEND ASP3_COMPILE_DEFS
    OMIT_ISTACK
    TOPPERS_OMIT_VECTOR_TABLE
)

function(asp3_set_pico_sdk_options TARGET)
  if (PICO_PLATFORM STREQUAL "rp2350-riscv")
    #  RISC-V（Hazard3）版：割込みコントローラは Xh3irq（カスタムCSR）
    #  ASP3 の chip_initialize() が mtvec を trap_vector_table に向けて掌握する．
    #  pico-sdk の irq_* は Xh3irq を経由しないため --wrap は不要．
    #  IRQ優先度初期化を抑止（ASP3 が Xh3irq を chip_initialize() で設定する）
    target_compile_definitions(${TARGET}
      PUBLIC PICO_RUNTIME_SKIP_INIT_PER_CORE_IRQ_PRIORITIES
    )
  else()
    #  ARM-S版：ASP3 が NVIC を掌握するため，SDKのベクタ/IRQ優先度初期化を抑止する
    target_compile_definitions(${TARGET}
      PUBLIC PICO_RUNTIME_NO_INIT_INSTALL_RAM_VECTOR_TABLE
      PUBLIC PICO_RUNTIME_SKIP_INIT_INSTALL_RAM_VECTOR_TABLE
      PUBLIC PICO_RUNTIME_SKIP_INIT_PER_CORE_IRQ_PRIORITIES
    )

    #  pico-sdk の割込み登録APIを ASP3 管理へ誘導（リンカ --wrap）
    target_link_options(${TARGET}
      PRIVATE "LINKER:--wrap=exception_get_vtable_handler"
      PRIVATE "LINKER:--wrap=exception_set_exclusive_handler"
      PRIVATE "LINKER:--wrap=exception_restore_handler"
      PRIVATE "LINKER:--wrap=irq_get_vtable_handler"
      PRIVATE "LINKER:--wrap=irq_set_exclusive_handler"
      PRIVATE "LINKER:--wrap=irq_add_shared_handler"
      PRIVATE "LINKER:--wrap=irq_remove_handler"
      PRIVATE "LINKER:--wrap=irq_add_tail_to_free_list"
    )
  endif()
endfunction()
