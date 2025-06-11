#include <stdint.h>
#include <string.h>
#include "em_device.h"
#include "drive_systick.h"


#define CALL_BACK_MAX        5  /* 最多5个回调 */

static systick_cb cb_array[CALL_BACK_MAX];

/** 单位ms */
static uint32_t systick_cnt = 0;

void SysTick_Handler(void)
{
    systick_cnt++;
    for(uint8_t i = 0; i < CALL_BACK_MAX; i++)
    {
        if(cb_array[i])
        {
            cb_array[i]();
        }
    }
}


void drive_systick_init(void)
{
    uint32_t load;

    load = (SystemCoreClock / 1000) - 1U;
    if (load > 0x00FFFFFFU)
    {
        return;
    }

    SysTick->CTRL =  SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD =  load;
    SysTick->VAL  =  0U;
    SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
    NVIC_EnableIRQ(SysTick_IRQn);

    for(int i = 0; i < CALL_BACK_MAX; i++)
    {
        cb_array[i] = NULL;
    }
}


uint8_t drive_systick_register(systick_cb cb)
{
    for(uint8_t i = 0; i < CALL_BACK_MAX; i++)
    {
        if(cb_array[i] == NULL)
        {
            cb_array[i] = cb;
            return i;
        }
    }
    return 0xFF;
}


uint32_t drive_systick_unregister(systick_cb cb)
{
    for(uint8_t i = 0; i < CALL_BACK_MAX; i++)
    {
        if(cb_array[i] == cb)
        {
            cb_array[i] = NULL;
            return 1;
        }
    }
    return 0;
}
