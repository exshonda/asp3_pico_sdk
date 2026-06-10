/*
 *  アセンブラ用マクロのターゲット依存部（RaspberryPi Pico 2 ASP3用）
 *  （ARM-M の rp2350 チップ層に chip_asm.inc は無いため，共通 arch の
 *   core_asm.inc を直接取り込む＝ベアメタル pico2_arm_gcc と同じ）
 */
#include "core_asm.inc"
