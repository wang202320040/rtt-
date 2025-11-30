/*
 * DS18B20温度传感器驱动
 * 单总线协议
 */
#include "drv_ds18b20.h"
#include "board.h"

/* 延时函数声明 */
extern void delay_us(uint32_t us);

/**
 * @brief 设置为输出模式
 */
static void ds18b20_set_output(void)
{
    rt_pin_mode(DS18B20_PIN, PIN_MODE_OUTPUT);
}

/**
 * @brief 设置为输入模式
 */
static void ds18b20_set_input(void)
{
    rt_pin_mode(DS18B20_PIN, PIN_MODE_INPUT);
}

/**
 * @brief 写高电平
 */
static void ds18b20_write_high(void)
{
    rt_pin_write(DS18B20_PIN, PIN_HIGH);
}

/**
 * @brief 写低电平
 */
static void ds18b20_write_low(void)
{
    rt_pin_write(DS18B20_PIN, PIN_LOW);
}

/**
 * @brief 读取引脚电平
 */
static uint8_t ds18b20_read_pin(void)
{
    return rt_pin_read(DS18B20_PIN);
}

/**
 * @brief 复位DS18B20
 */
void ds18b20_reset(void)
{
    ds18b20_set_output();
    ds18b20_write_low();
    delay_us(750);
    ds18b20_write_high();
    delay_us(15);
}

/**
 * @brief 检测DS18B20是否存在
 */
uint8_t ds18b20_check(void)
{
    uint8_t retry = 0;

    ds18b20_set_input();
    while (ds18b20_read_pin() && retry < 200)
    {
        retry++;
        delay_us(1);
    }
    if (retry >= 200) return 1;

    retry = 0;
    while (!ds18b20_read_pin() && retry < 240)
    {
        retry++;
        delay_us(1);
    }
    if (retry >= 240) return 1;

    return 0;
}

/**
 * @brief 读取一个位
 */
uint8_t ds18b20_read_bit(void)
{
    uint8_t data;

    ds18b20_set_output();
    ds18b20_write_low();
    delay_us(2);
    ds18b20_write_high();
    ds18b20_set_input();
    delay_us(12);
    data = ds18b20_read_pin();
    delay_us(50);

    return data;
}

/**
 * @brief 读取一个字节
 */
uint8_t ds18b20_read_byte(void)
{
    uint8_t i, data = 0;

    for (i = 0; i < 8; i++)
    {
        data >>= 1;
        if (ds18b20_read_bit())
            data |= 0x80;
    }

    return data;
}

/**
 * @brief 写入一个字节
 */
void ds18b20_write_byte(uint8_t dat)
{
    uint8_t i;

    ds18b20_set_output();
    for (i = 0; i < 8; i++)
    {
        if (dat & 0x01)
        {
            ds18b20_write_low();
            delay_us(2);
            ds18b20_write_high();
            delay_us(60);
        }
        else
        {
            ds18b20_write_low();
            delay_us(60);
            ds18b20_write_high();
            delay_us(2);
        }
        dat >>= 1;
    }
}

/**
 * @brief 启动温度转换
 */
void ds18b20_start(void)
{
    ds18b20_reset();
    ds18b20_check();
    ds18b20_write_byte(0xCC);  /* 跳过ROM */
    ds18b20_write_byte(0x44);  /* 启动转换 */
}

/**
 * @brief 获取温度值
 * @return 温度值 (放大10倍，如365表示36.5度)
 */
int16_t ds18b20_get_temp(void)
{
    uint8_t temp_l, temp_h;
    int16_t temp;

    ds18b20_start();
    ds18b20_reset();
    ds18b20_check();
    ds18b20_write_byte(0xCC);  /* 跳过ROM */
    ds18b20_write_byte(0xBE);  /* 读暂存器 */

    temp_l = ds18b20_read_byte();
    temp_h = ds18b20_read_byte();

    temp = (int16_t)((temp_h << 8) | temp_l);

    /* 转换为温度值 (放大10倍) */
    if (temp < 0)
        temp = -((~temp + 1) * 0.625f);
    else
        temp = temp * 0.625f;

    return temp;
}

/**
 * @brief 初始化DS18B20
 */
int ds18b20_init(void)
{
    rt_pin_mode(DS18B20_PIN, PIN_MODE_OUTPUT);
    ds18b20_reset();

    if (ds18b20_check())
    {
        rt_kprintf("DS18B20: Device not found!\n");
        return -RT_ERROR;
    }

    rt_kprintf("DS18B20: Initialized successfully\n");
    return RT_EOK;
}

INIT_DEVICE_EXPORT(ds18b20_init);
