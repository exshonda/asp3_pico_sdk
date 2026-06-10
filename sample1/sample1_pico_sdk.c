#include <stdio.h>
#include "pico/stdlib.h"
#include "kernel.h"

int main()
{
    stdio_init_all();

#if 0
    while(!stdio_usb_connected())
        Asm("WFI");;
#endif

    /* カーネル起動前に割込みを禁止（ISA 依存） */
#if defined(__ARM_ARCH)
    Asm("cpsid f":::"memory");          /* ARM-M: FIQ 禁止 */
#else
    Asm("csrci mstatus, 8":::"memory"); /* RISC-V: mstatus.MIE クリア */
#endif

    sta_ker();

    return 0;
}
