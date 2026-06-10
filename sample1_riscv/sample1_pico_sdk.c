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

    Asm("csrci mstatus, 8":::"memory");  /* RISC-V: clear MIE in mstatus before starting kernel */

    sta_ker();

    return 0;
}
