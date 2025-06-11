#ifndef __DRIVE_SYSTICK_H__
#define __DRIVE_SYSTICK_H__


typedef void (*systick_cb)(void);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn 初始化一个1ms的定时器
 *
 * @param void
 *
 * @return void
 */
void drive_systick_init(void);

/**
 * @fn 向定时器注册一个回调函数
 *
 * @param cb 函数指针，指向被调用的函数
 *
 * @return 如果注册成功将返回cb对应的ID，0xFF表示注册失败
 */
uint8_t drive_systick_register(systick_cb cb);

/**
 * @fn 取消注册
 *
 * @param cb 函数指针，根据函数指针来查找
 *
 * @return 错误码 @ref lvs_error_t
 */
uint32_t drive_systick_unregister(systick_cb cb);

#ifdef __cplusplus
}
#endif

#endif
