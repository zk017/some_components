#include "mykey.h"
#include "log.h"
#include "boards.h"

//static uint32_t g_button_time = 0;

static ButtonState buttons[BUTTON_MAX];
// 配置按键回调函数
void Button_SetCallback(ButtonId button, ButtonCallback callback)
{
    if (button < BUTTON_MAX)
    {
        buttons[button].callback = callback;
    }
}

// 读取按键电平状态（示例函数，需根据硬件实现）
static bool ReadButtonState(ButtonId button)
{
    // 这里应根据实际硬件连接实现读取按键电平
    // 示例返回固定值，实际应使用GPIO读取函数
    bool ret = button_is_pressed(button);
    return ret;
}

// 获取当前时间（毫秒）
static uint32_t g_button_ticks = 0;
static uint32_t GetCurrentTime(void)
{

    return g_button_ticks++;
}

#define DEBUG_LOG(fmt, ...) LOGD(fmt, ##__VA_ARGS__)
//#define DEBUG_LOG(fmt, ...)     //LOGD("")

#if 1
void Button_Scan(void)
{
    uint32_t currentTime = GetCurrentTime();
    bool bothPressed = buttons[BUTTON_1].isPressed && buttons[BUTTON_2].isPressed;
    static bool bothPressTriggered = false;

    // 阶段1：更新按键状态
    for (int i = 0; i < BUTTON_MAX; i++)
    {
        ButtonId btn = (ButtonId)i;
        ButtonState *state = &buttons[btn];
        state->lastState = state->isPressed;
        state->isPressed = ReadButtonState(btn);

        if (state->isPressed && !state->lastState)   // 按下事件
        {
            uint32_t interval = currentTime - state->lastClickTime;

            // 双击窗口期逻辑
            if (interval > 500 || interval == 0) // 添加interval==0的情况，处理首次点击
            {
                state->clickCount = 1;
                state->doubleClickWindow = true;
            }
            else
            {
                state->clickCount++;
            }

            state->pressStartTime = currentTime;
            state->lastClickTime = currentTime;
            state->suppressShortPress = false;
            state->isDoubleReady = false;
            state->longPressTriggered = false;

            //DEBUG_LOG("Btn%d Press, Count:%d, Interval:%ums\n", btn, state->clickCount, interval);
        }

        if (!state->isPressed && state->lastState)   // 释放事件
        {
            state->lastReleaseTime = currentTime;
            state->isDoubleReady = true;

            //DEBUG_LOG("Btn%d Release, Time:%ums\n", btn, currentTime);
        }
    }

    // 阶段2：两键事件检测
    if (bothPressed)
    {
        if (!bothPressTriggered)
        {
            bothPressTriggered = true;
            uint32_t diff = currentTime - buttons[BUTTON_1].pressStartTime;
            diff = (diff > currentTime - buttons[BUTTON_2].pressStartTime) ?
                   (currentTime - buttons[BUTTON_2].pressStartTime) : diff;
            if (diff < 50)
            {
                buttons[BUTTON_1].inTwoButtonSequence = true;
                buttons[BUTTON_2].inTwoButtonSequence = true;
                buttons[BUTTON_1].suppressShortPress = true;
                buttons[BUTTON_2].suppressShortPress = true;
                if (buttons[BUTTON_1].callback) buttons[BUTTON_1].callback(BUTTON_1, EVENT_TWO_BUTTONS_PRESS);
                if (buttons[BUTTON_2].callback) buttons[BUTTON_2].callback(BUTTON_2, EVENT_TWO_BUTTONS_PRESS);
            }
        }
    }
    else
    {
        bothPressTriggered = false;
        if (!buttons[BUTTON_1].isPressed && !buttons[BUTTON_2].isPressed)
        {
            buttons[BUTTON_1].inTwoButtonSequence = false;
            buttons[BUTTON_2].inTwoButtonSequence = false;
        }
    }

    // 阶段3：单键事件检测 - 重新设计事件检测逻辑
    for (int i = 0; i < BUTTON_MAX; i++)
    {
        ButtonId btn = (ButtonId)i;
        ButtonState *state = &buttons[btn];

        // 长按检测
        if (state->isPressed && !state->inTwoButtonSequence && !state->longPressTriggered)
        {
            uint32_t pressDuration = currentTime - state->pressStartTime;
            if (pressDuration >= 1000)
            {
                if (state->callback) state->callback(btn, EVENT_LONG_PRESS);
                state->suppressShortPress = true;
                state->longPressTriggered = true;
                state->doubleClickWindow = false; // 长按触发后关闭双击窗口
                //DEBUG_LOG("Btn%d Long Press!\n", btn);
            }
        }

        // 双击窗口期超时检测
        if (state->doubleClickWindow && !state->isPressed &&
            currentTime - state->lastClickTime > 500)
        {
            state->doubleClickWindow = false;

            // 处理单击事件
            if (state->clickCount == 1 && !state->suppressShortPress)
            {
                uint32_t pressDuration = state->lastReleaseTime - state->pressStartTime;
                if (pressDuration < 500)
                {
                    if (state->callback) state->callback(btn, EVENT_SHORT_PRESS);
                    //DEBUG_LOG("Btn%d Short Press!\n", btn);
                }
            }

            // 重置单击计数
            state->clickCount = 0;
            state->lastClickTime = 0;
        }

        // 双击检测 - 仅在窗口期内且连续两次点击时触发
        if (state->clickCount == 2 && state->doubleClickWindow &&
            !state->isPressed && !state->suppressShortPress)
        {
            if (state->callback) state->callback(btn, EVENT_DOUBLE_CLICK);
            //DEBUG_LOG("Btn%d Double Click!\n", btn);

            // 重置双击相关状态
            state->clickCount = 0;
            state->lastClickTime = 0;
            state->doubleClickWindow = false;
        }

        // 释放超过500ms后重置所有状态
        if (!state->isPressed)
        {
            uint32_t releaseDur = currentTime - state->lastReleaseTime;
            if (releaseDur > 500)
            {
                state->suppressShortPress = false;
                state->isDoubleReady = false;
                state->clickCount = 0;
                state->lastClickTime = 0;
                //DEBUG_LOG("Btn%d Reset all states\n", btn);
            }
        }
    }
}


#else
void Button_Scan(void)
{
    uint32_t currentTime = GetCurrentTime();
    bool bothPressed = buttons[BUTTON_1].isPressed && buttons[BUTTON_2].isPressed;
    static bool bothPressTriggered = false;

    // 阶段1：更新按键状态
    for (int i = 0; i < BUTTON_MAX; i++)
    {
        ButtonId btn = (ButtonId)i;
        ButtonState *state = &buttons[btn];
        state->lastState = state->isPressed;
        state->isPressed = ReadButtonState(btn);

        if (state->isPressed && !state->lastState)   // 按下事件
        {
            uint32_t interval = currentTime - state->lastClickTime;

            // 双击窗口期逻辑（新增）
            if (interval > 500)
            {
                state->clickCount = 1;
                state->doubleClickWindow = true;
            }
            else
            {
                state->clickCount++;
            }

            state->pressStartTime = currentTime;
            state->lastClickTime = currentTime;
            state->suppressShortPress = false;
            state->isDoubleReady = false;
            state->longPressTriggered = false;

            DEBUG_LOG("Btn %d Press, Count:%d, Interval:%ums\n", btn, state->clickCount, interval);
        }

        if (!state->isPressed && state->lastState)   // 释放事件
        {
            state->lastReleaseTime = currentTime;
            state->isDoubleReady = true;

            // 双击窗口期超时检测（新增）
            if (state->doubleClickWindow && currentTime - state->lastClickTime > 500)
            {
                state->doubleClickWindow = false;
            }

            //DEBUG_LOG("Btn %d Release, Time:%ums\n", btn, currentTime);
        }
    }

    // 阶段2：两键事件检测
    if (bothPressed)
    {
        if (!bothPressTriggered)
        {
            bothPressTriggered = true;
            uint32_t diff = currentTime - buttons[BUTTON_1].pressStartTime;
            diff = (diff > currentTime - buttons[BUTTON_2].pressStartTime) ?
                   (currentTime - buttons[BUTTON_2].pressStartTime) : diff;
            if (diff < 50)
            {
                buttons[BUTTON_1].inTwoButtonSequence = true;
                buttons[BUTTON_2].inTwoButtonSequence = true;
                buttons[BUTTON_1].suppressShortPress = true;
                buttons[BUTTON_2].suppressShortPress = true;
                if (buttons[BUTTON_1].callback) buttons[BUTTON_1].callback(BUTTON_1, EVENT_TWO_BUTTONS_PRESS);
                if (buttons[BUTTON_2].callback) buttons[BUTTON_2].callback(BUTTON_2, EVENT_TWO_BUTTONS_PRESS);
            }
        }
    }
    else
    {
        bothPressTriggered = false;
        if (!buttons[BUTTON_1].isPressed && !buttons[BUTTON_2].isPressed)
        {
            buttons[BUTTON_1].inTwoButtonSequence = false;
            buttons[BUTTON_2].inTwoButtonSequence = false;
        }
    }

    // 阶段3：单键事件检测
    for (int i = 0; i < BUTTON_MAX; i++)
    {
        ButtonId btn = (ButtonId)i;
        ButtonState *state = &buttons[btn];

        // 长按检测（优化）
        if (state->isPressed && !state->inTwoButtonSequence && !state->longPressTriggered)
        {
            uint32_t pressDuration = currentTime - state->pressStartTime;
            if (pressDuration >= 1000)
            {
                if (state->callback) state->callback(btn, EVENT_LONG_PRESS);
                state->suppressShortPress = true;
                state->longPressTriggered = true;
                state->doubleClickWindow = false; // 长按触发后关闭双击窗口
                DEBUG_LOG("Btn %d Long Press!\n", btn);
            }
        }

        // 双击检测（优化逻辑）
        if (state->isDoubleReady && state->clickCount >= 2 &&
                state->doubleClickWindow && !state->inTwoButtonSequence && !state->suppressShortPress)
        {
            if (state->callback) state->callback(btn, EVENT_DOUBLE_CLICK);
            DEBUG_LOG("Btn %d Double Click!\n", btn);

            // 重置双击相关状态
            state->clickCount = 0;
            state->lastClickTime = 0;
            state->isDoubleReady = false;
            state->doubleClickWindow = false;
        }

        // 短按检测（仅在双击窗口外触发）
        if (state->isDoubleReady && state->clickCount == 1 &&
                !state->doubleClickWindow && !state->inTwoButtonSequence && !state->suppressShortPress)
        {
            uint32_t duration = currentTime - state->pressStartTime;
            if (duration < 500)
            {
                if (state->callback) state->callback(btn, EVENT_SHORT_PRESS);
                DEBUG_LOG("Btn %d Short Press!\n", btn);
                state->clickCount = 0;
                state->lastClickTime = 0;
                state->isDoubleReady = false;
            }
        }

        // 释放超过500ms后重置部分状态
        if (!state->isPressed)
        {
            uint32_t releaseDur = currentTime - state->lastReleaseTime;
            if (releaseDur > 500)
            {
                state->suppressShortPress = false;
                state->isDoubleReady = false;
                if (!state->doubleClickWindow)   // 仅在双击窗口关闭时重置计数
                {
                    state->clickCount = 0;
                    state->lastClickTime = 0;
                }
                //DEBUG_LOG("Btn%d Reset partial states\n", btn);
            }
        }
    }
}
#endif

// 示例回调函数
void ButtonEventCallback(ButtonId btn, ButtonEventType event)
{
    const char *evtName[] = {"NONE", "SHORT", "LONG", "DOUBLE", "TWO"};
    LOGD("Event: %s on Button %d\n", evtName[event], btn);
}

