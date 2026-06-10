
list(APPEND ASP3_SYMVAL_TABLES
    ${ARCHDIR}/common/core_sym.def
)

list(APPEND ASP3_OFFSET_TRB_FILES
    ${ARCHDIR}/common/core_offset.py
)

list(APPEND ASP3_INCLUDE_DIRS
    ${CHIPDIR}
    ${ARCHDIR}/common
    ${GCCDIR}
)

list(APPEND ASP3_COMPILE_DEFS
    TOPPERS_CORTEX_M33
    __TARGET_ARCH_THUMB=5
    __TARGET_FPU_FPV4_SP
    TOPPERS_ENABLE_TRUSTZONE
    TBITW_IPRI=4
)

list(APPEND ASP3_ARCH_C_FILES
    ${ARCHDIR}/common/core_kernel_impl.c
    ${ARCHDIR}/common/core_support.S
)

if(NOT DEFINED TOPPERS_OMIT_CHIP_SERIAL)
    list(APPEND ASP3_ARCH_C_FILES
        ${CHIPDIR}/chip_serial.c
    )
    set(ARCH_SERIAL ${CHIPDIR}/chip_serial.c)
else()
    set(ARCH_SERIAL "")
endif()
