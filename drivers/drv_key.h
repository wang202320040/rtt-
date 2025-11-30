/*
 * 按键和蜂鸣器驱动
 */
#ifndef __DRV_KEY_H__
#define __DRV_KEY_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO引脚定义 */
#define KEY0_PIN        GET_PIN(B, 12)
#define KEY1_PIN        GET_PIN(B, 13)
#define KEY2_PIN        GET_PIN(B, 14)
#define KEY3_PIN        GET_PIN(B, 15)
#define KEY4_PIN        GET_PIN(A, 8)
#define BEEP_PIN        GET_PIN(B, 9)

/* 按键值定义 */
#define KEY_NONE        0
#define KEY0_PRESS      1
#define KEY1_PRESS      2
#define KEY2_PRESS      3
#define KEY3_PRESS      4
#define KEY4_PRESS      5

/* 函数声明 */
int key_init(void);
uint8_t key_scan(uint8_t mode);
void beep_on(void);
void beep_off(void);
void beep_toggle(void);
void beep_beep(uint16_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_KEY_H__ */
