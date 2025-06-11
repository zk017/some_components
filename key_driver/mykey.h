#ifndef __MYKEY__H
#define __MYKEY_H

#include "stdint.h"
#include "stdbool.h"

typedef enum
{
    BUTTON_1 = 0,
    BUTTON_2 = 1,
    BUTTON_MAX
} ButtonId;


// 按键事件类型
typedef enum
{
    EVENT_NONE = 0,
    EVENT_SHORT_PRESS,
    EVENT_LONG_PRESS,
    EVENT_DOUBLE_CLICK,
    EVENT_TWO_BUTTONS_PRESS
} ButtonEventType;

// 按键事件回调函数类型
typedef void (*ButtonCallback)(ButtonId button, ButtonEventType event);

// 按键状态结构体
typedef struct
{
    bool isPressed;                // 当前按下状态
    bool lastState;                // 上次状态
    uint32_t pressStartTime;       // 按下开始时间
    uint32_t lastReleaseTime;      // 上次释放时间
    uint8_t clickCount;            // 点击次数
    uint32_t lastClickTime;        // 上次点击时间
    ButtonCallback callback;       // 回调函数
    bool inTwoButtonSequence;      // 是否参与两键事件序列
    bool suppressShortPress;       // 抑制短按事件标志
    bool resetDoubleClick;         // 重置双击检测标志
    bool isDoubleReady;            // 双击检测就绪标志
    bool longPressTriggered;       // 长按是否已触发（新增）
    bool doubleClickWindow;        // 双击窗口期（新增）
} ButtonState;

// 全局按键状态数组


void Button_Scan(void);
void Button_SetCallback(ButtonId button, ButtonCallback callback);
void ButtonEventCallback(ButtonId button, ButtonEventType event);
#endif
