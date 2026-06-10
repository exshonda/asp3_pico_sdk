# -*- coding: utf-8 -*-
#
#   TOPPERS/ASP Kernel
#       Toyohashi Open Platform for Embedded Real-Time Systems/
#       Advanced Standard Profile Kernel
#
#   $Id: target_kernel.py (converted from target_kernel.trb) $
#

#
#  パス2のターゲット依存テンプレート（RaspberryPi Pico 2 ASP3用）
#

#
#  有効な割込み番号，割込みハンドラ番号
#
INTNO_VALID = list(range(15, TMAX_INTNO + 1))
INHNO_VALID = INTNO_VALID

#
#  有効なCPU例外番号
#
EXCNO_VALID = [2, 3, 4, 5, 6, 7, 12]

#
#  CRE_ISRで使用できる割込み番号とそれに対応する割込みハンドラ番号
#
INTNO_CREISR_VALID = INTNO_VALID
INHNO_CREISR_VALID = INHNO_VALID

#
#  DEF_INT／DEF_EXCで使用できる割込みハンドラ番号／CPU例外ハンドラ番号
#
INHNO_DEFINH_VALID = INHNO_VALID
EXCNO_DEFEXC_VALID = EXCNO_VALID

#
#  CFG_INTで使用できる割込み番号と割込み優先度
#
INTNO_CFGINT_VALID = INTNO_VALID
INTPRI_CFGINT_VALID = list(range(-(1 << TBITW_IPRI), 0))

#
#  TSKINICTXBの初期化情報を生成
#
def GenerateTskinictxb(key, params):
    return ("{" +
            f"\t(void *)({params['tinib_stk']}), " +
            f"\t((void *)((char *)({params['tinib_stk']}) + " +
            f"({params['tinib_stksz']}))), " +
            "}")


#
#  カーネルのデータ領域のセクション名
#
def SecnameKernelData(cls):
    return ""


#
#  配置するセクションを指定した変数定義の生成
#
def DefineVariableSection(gen_file, defvar, secname):
    if secname != "":
        gen_file.add(f'{defvar} __attribute__((section("{secname}"),nocommon));')
    else:
        gen_file.add(f"{defvar};")


#
#  スタック領域のセクション名
#
def SecnameStack(cls):
    return ""


#
#  標準テンプレートファイルのインクルード
#
IncludeTrb("kernel/kernel.py")

#
#  OMIT_ISTACKが定義されている場合，kernel.pyは_kernel_istk/_kernel_istkptを
#  生成しないが，core_support.Sのstart_dispatchは常にこれらを参照する．
#
if OMIT_ISTACK:
    kernelCfgC.comment_header("Stack Area for Non-task Context (pico-sdk integration)")
    kernelCfgC.add("static STK_T _kernel_istack[COUNT_STK_T(DEFAULT_ISTKSZ)];")
    kernelCfgC.add("const size_t _kernel_istksz = ROUND_STK_T(DEFAULT_ISTKSZ);")
    kernelCfgC.add("STK_T *const _kernel_istk = _kernel_istack;")
    kernelCfgC.add("#ifdef TOPPERS_ISTKPT")
    kernelCfgC.add("STK_T *const _kernel_istkpt = TOPPERS_ISTKPT(_kernel_istack, ROUND_STK_T(DEFAULT_ISTKSZ));")
    kernelCfgC.add("#endif /* TOPPERS_ISTKPT */")
    kernelCfgC.add()

#
#  ISRスタブの生成（pico-sdk のWEAK isr_irqN をオーバーライドする）
#  割込みエントリ（isr_irqN のaliasターゲットとして同一ファイルに生成する）
#  GCCのalias属性は同一翻訳単位のシンボルのみを対象とするため，
#  このラッパーを kernel_cfg.c 内に定義する．
#
kernelCfgC.add("extern void _kernel_core_int_entry(void);")
kernelCfgC.add("void _kernel_asp3_int_entry(void) { _kernel_core_int_entry(); }")
kernelCfgC.add()

for _index in range(len(INTNO_VALID)):
    if _index == 0:
        _isr_name = "isr_systick"
    else:
        _isr_name = f"isr_irq{_index - 1}"
    _inhno = INTNO_VALID[_index]
    _inh = cfgData["DEF_INH"].get(_inhno)
    if _inh and (_inh["inhatr"] & TA_NONKERNEL) != 0:
        kernelCfgC.add(f"void {_isr_name}(void)"
                       f" {{ (((void (*)(void)))({_inh['inthdr']}))(); }}"
                       f" /* {_inhno} */")
    else:
        kernelCfgC.add(f'void {_isr_name}(void)'
                       f' __attribute__((alias("_kernel_asp3_int_entry")));'
                       f' /* {_inhno} */')
kernelCfgC.add()

#
#  _kernel_c_exc_tbl（ROM版，初期化データ）の生成
#  インデックス = IPSR値
#    0..14  : CPU例外（index = IPSR = 例外番号）
#    15..TMAX_INTNO : 割込み（index = IPSR = INTNO）
#
kernelCfgC.add("const FP _kernel_c_exc_tbl[] = {")
for _excno in range(15):
    _exc = cfgData["DEF_EXC"].get(_excno)
    if _exc:
        kernelCfgC.add(f"   (FP)({_exc['exchdr']}), /* {_excno} */")
    else:
        kernelCfgC.add(f"   (FP)(_kernel_default_exc_handler), /* {_excno} */")
for _inhno in INTNO_VALID:
    _inh = cfgData["DEF_INH"].get(_inhno)
    if _inh:
        kernelCfgC.add(f"   (FP)({_inh['inthdr']}), /* {_inhno} */")
    else:
        kernelCfgC.add(f"   (FP)(_kernel_default_int_handler), /* {_inhno} */")
kernelCfgC.add2("};")

#
#  _kernel_exc_tbl（RAM版，.ram_vector_table セクション）の宣言
#
kernelCfgC.add2('FP __attribute__((section(".ram_vector_table"),aligned(0x100)))'
                ' _kernel_exc_tbl[TMAX_INTNO];')

#
#  core_kernel_impl.c の core_initialize() が参照するダミーのベクタテーブル
#
kernelCfgC.add2("const FP _kernel_vector_table[1] = {(FP)0};")

#
#  _kernel_bitpat_cfgintの生成
#
if (TMAX_INTNO & 0x0f) == 0x00:
    _bitpat_cfgint_num = TMAX_INTNO >> 4
else:
    _bitpat_cfgint_num = (TMAX_INTNO >> 4) + 1

kernelCfgC.add()
kernelCfgC.add(f"const uint32_t _kernel_bitpat_cfgint[{_bitpat_cfgint_num}] = {{")
for _num in range(_bitpat_cfgint_num):
    _bitpat = 0
    for _inhno in range(_num * 32, _num * 32 + 32):
        for _, _inh_params in cfgData["DEF_INH"].items():
            if int(_inh_params["inhno"]) == _inhno:
                _bitpat |= (1 << (_inhno & 0x1f))
    kernelCfgC.add(f"   UINT32_C(0x{_bitpat:08x}),")
kernelCfgC.add2("};")
