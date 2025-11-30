/*
 * 按键和蜂鸣器驱动
 */
#include "drv_key.h"

/**
 * @brief 蜂鸣器开
 */
void beep_on(void)
{
    rt_pin_write(BEEP_PIN, PIN_HIGH);
}

/**
 * @brief 蜂鸣器关
 */
void beep_off(void)
{
    rt_pin_write(BEEP_PIN, PIN_LOW);
}

/**
 * @brief 蜂鸣器翻转
 */
void beep_toggle(void)
{
    static uint8_t state = 0;
    state = !state;
    rt_pin_write(BEEP_PIN, state ? PIN_HIGH : PIN_LOW);
}

/**
 * @brief 蜂鸣器响一段时间
 */
void beep_beep(uint16_t ms)
{
    beep_on();
    rt_thread_mdelay(ms);
    beep_off();
}

/**
 * @brief 按键扫描
 * @param mode 0-不支持连续按; 1-支持连续按
 * @return 按键值
 */
uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;
    uint8_t key_val = KEY_NONE;

    if (mode) key_up = 1;

    if (key_up && (rt_pin_read(KEY0_PIN) == PIN_LOW ||
                   rt_pin_read(KEY1_PIN) == PIN_LOW ||
                   rt_pin_read(KEY2_PIN) == PIN_LOW ||
                   rt_pin_read(KEY3_PIN) == PIN_LOW ||
                   rt_pin_read(KEY4_PIN) == PIN_LOW))
    {
        rt_thread_mdelay(10);  /* 消抖 */
        key_up = 0;

        if (rt_pin_read(KEY0_PIN) == PIN_LOW)
            key_val = KEY0_PRESS;
        else if (rt_pin_read(KEY1_PIN) == PIN_LOW)
            key_val = KEY1_PRESS;
        else if (rt_pin_read(KEY2_PIN) == PIN_LOW)
            key_val = KEY2_PRESS;
        else if (rt_pin_read(KEY3_PIN) == PIN_LOW)
            key_val = KEY3_PRESS;
        else if (rt_pin_read(KEY4_PIN) == PIN_LOW)
            key_val = KEY4_PRESS;
    }
    else if (rt_pin_read(KEY0_PIN) == PIN_HIGH &&
             rt_pin_read(KEY1_PIN) == PIN_HIGH &&
             rt_pin_read(KEY2_PIN) == PIN_HIGH &&
             rt_pin_read(KEY3_PIN) == PIN_HIGH &&
             rt_pin_read(KEY4_PIN) == PIN_HIGH)
    {
        key_up = 1;
    }

    return key_val;
}

/**
 * @brief 初始化按键和蜂鸣器
 */
int key_init(void)
{
    /* 配置按键引脚为上拉输入 */
    rt_pin_mode(KEY0_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY1_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY2_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY3_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY4_PIN, PIN_MODE_INPUT_PULLUP);

    /* 配置蜂鸣器引脚为输出 */
    rt_pin_mode(BEEP_PIN, PIN_MODE_OUTPUT);
    beep_off();

    rt_kprintf("KEY & BEEP: Initialized successfully\n");
    return RT_EOK;
}

INIT_DEVICE_EXPORT(key_init);
