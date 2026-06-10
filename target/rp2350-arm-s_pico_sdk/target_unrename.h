/* This file is generated from target_rename.def by genrename. */

#ifdef TOPPERS_TARGET_RENAME_H
#undef TOPPERS_TARGET_RENAME_H

/*
 *  target_config.c
 */
#undef target_initialize
#undef target_exit

#ifdef TOPPERS_LABEL_ASM
#undef _target_initialize
#undef _target_exit
#endif /* TOPPERS_LABEL_ASM */

/* pico-sdk overridesを元に戻す（core_unrename.hで#undefされる） */
#undef core_exc_entry
#undef svc_handler
#undef pendsv_handler

#include "chip_unrename.h"

#endif /* TOPPERS_TARGET_RENAME_H */
