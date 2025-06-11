#include "drive_systick.h"

static void key_scan_handler(void)
{
    Button_Scan();
}


//===============================================================
//EFR32BG21  1ms timer.
//---------------------------------------------------------------
volatile uint32_t msTickCount;
uint32_t UTIL_init( void )
{

	uint32_t stat;
	uint32_t ticks;

	/* Setup SysTick Timer for 1 msec interrupts  */
	ticks = CMU_ClockFreqGet( cmuClock_CORE ) / 1000;
	stat = SysTick_Config( ticks );

	return stat;
}
//--------------------------------------------------------------------------
void SysTick_Handler( void ) //1ms one time name can't change.
{
	static uint16_t i = 0;
    msTickCount++;
    i++;

    key_scan_handler();

    /*
   if(i>=1000)
   {
	   i = 0;
	   LOGD("10ms passed!");
   }
	*/
   return;

}
//==============================================================================
void main(void)
{
    Button_SetCallback(BUTTON_1, ButtonEventCallback); //@zk20250611
    Button_SetCallback(BUTTON_2, ButtonEventCallback);
#if mothod_one
    drive_systick_init();
    drive_systick_register(key_scan_handler);
#else //method 2
    UTIL_init();
#endif
}