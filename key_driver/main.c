#include "drive_systick.h"

static void key_scan_handler(void)
{
    Button_Scan();
}

void main(void)
{
    drive_systick_init();
    drive_systick_register(key_scan_handler);
}