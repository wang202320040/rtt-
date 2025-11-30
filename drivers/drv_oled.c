/*
 * OLED I2C驱动 - SSD1306 128x64
 * 基于RT-Thread I2C框架
 */
#include "drv_oled.h"
#include "oled_font.h"

/* I2C设备句柄 */
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

/**
 * @brief I2C写入一个字节
 */
static rt_err_t oled_write_byte(uint8_t reg, uint8_t data)
{
    struct rt_i2c_msg msgs;
    uint8_t buf[2];

    buf[0] = reg;
    buf[1] = data;

    msgs.addr  = OLED_I2C_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf   = buf;
    msgs.len   = 2;

    if (rt_i2c_transfer(i2c_bus, &msgs, 1) == 1)
        return RT_EOK;
    else
        return -RT_ERROR;
}

/**
 * @brief 写命令
 */
static void oled_write_cmd(uint8_t cmd)
{
    oled_write_byte(0x00, cmd);
}

/**
 * @brief 写数据
 */
static void oled_write_data(uint8_t data)
{
    oled_write_byte(0x40, data);
}

/**
 * @brief OLED初始化
 */
int oled_init(void)
{
    /* 查找I2C总线 */
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(OLED_I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("OLED: Can't find %s device!\n", OLED_I2C_BUS_NAME);
        return -RT_ERROR;
    }

    rt_thread_mdelay(100);

    /* 初始化命令序列 */
    oled_write_cmd(0xAE);  /* 关闭显示 */
    oled_write_cmd(0x20);  /* 设置内存地址模式 */
    oled_write_cmd(0x10);  /* 页地址模式 */
    oled_write_cmd(0xB0);  /* 设置页起始地址 */
    oled_write_cmd(0xC8);  /* COM输出扫描方向 */
    oled_write_cmd(0x00);  /* 设置低列地址 */
    oled_write_cmd(0x10);  /* 设置高列地址 */
    oled_write_cmd(0x40);  /* 设置起始行地址 */
    oled_write_cmd(0x81);  /* 设置对比度控制 */
    oled_write_cmd(0xFF);  /* 亮度 0x00~0xFF */
    oled_write_cmd(0xA1);  /* 段重映射 */
    oled_write_cmd(0xA6);  /* 正常显示 */
    oled_write_cmd(0xA8);  /* 设置复用比 */
    oled_write_cmd(0x3F);  /* 1/64 */
    oled_write_cmd(0xA4);  /* 输出跟随RAM内容 */
    oled_write_cmd(0xD3);  /* 设置显示偏移 */
    oled_write_cmd(0x00);  /* 无偏移 */
    oled_write_cmd(0xD5);  /* 设置显示时钟分频/振荡频率 */
    oled_write_cmd(0xF0);  /* 分频比 */
    oled_write_cmd(0xD9);  /* 设置预充电周期 */
    oled_write_cmd(0x22);
    oled_write_cmd(0xDA);  /* 设置COM引脚硬件配置 */
    oled_write_cmd(0x12);
    oled_write_cmd(0xDB);  /* 设置VCOMH */
    oled_write_cmd(0x20);  /* 0.77xVcc */
    oled_write_cmd(0x8D);  /* 设置DC-DC使能 */
    oled_write_cmd(0x14);
    oled_write_cmd(0xAF);  /* 开启显示 */

    oled_clear();

    rt_kprintf("OLED: Initialized successfully\n");
    return RT_EOK;
}

/**
 * @brief 设置坐标
 */
void oled_set_pos(uint8_t x, uint8_t y)
{
    oled_write_cmd(0xB0 + y);
    oled_write_cmd(((x & 0xF0) >> 4) | 0x10);
    oled_write_cmd((x & 0x0F) | 0x01);
}

/**
 * @brief 填充屏幕
 */
void oled_fill(uint8_t data)
{
    uint8_t m, n;
    for (m = 0; m < 8; m++)
    {
        oled_write_cmd(0xB0 + m);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
        for (n = 0; n < 128; n++)
        {
            oled_write_data(data);
        }
    }
}

/**
 * @brief 清屏
 */
void oled_clear(void)
{
    oled_fill(0x00);
}

/**
 * @brief 开启显示
 */
void oled_on(void)
{
    oled_write_cmd(0x8D);
    oled_write_cmd(0x14);
    oled_write_cmd(0xAF);
}

/**
 * @brief 关闭显示
 */
void oled_off(void)
{
    oled_write_cmd(0x8D);
    oled_write_cmd(0x10);
    oled_write_cmd(0xAE);
}

/**
 * @brief 显示一个字符
 * @param x: 0~127
 * @param y: 0~7
 * @param chr: 字符
 * @param size: 字体大小 (1:6x8, 2:8x16)
 * @param mode: 0正常, 1反显
 */
void oled_show_char(uint8_t x, uint8_t y, char chr, uint8_t size, uint8_t mode)
{
    uint8_t c = 0, i = 0;

    c = chr - ' ';

    if (size == 2)
    {
        if (x > 120) { x = 0; y++; }
        oled_set_pos(x, y);
        for (i = 0; i < 8; i++)
        {
            if (mode == 1)
                oled_write_data(~F8X16[c * 16 + i]);
            else
                oled_write_data(F8X16[c * 16 + i]);
        }
        oled_set_pos(x, y + 1);
        for (i = 0; i < 8; i++)
        {
            if (mode == 1)
                oled_write_data(~F8X16[c * 16 + i + 8]);
            else
                oled_write_data(F8X16[c * 16 + i + 8]);
        }
    }
}

/**
 * @brief 显示字符串
 */
void oled_show_string(uint8_t x, uint8_t y, const char *str, uint8_t size)
{
    uint8_t j = 0;

    while (str[j] != '\0')
    {
        if (size == 2)
        {
            uint8_t c = str[j] - 32;
            if (x > 120) { x = 0; y++; }
            oled_set_pos(x, y);
            for (uint8_t i = 0; i < 8; i++)
                oled_write_data(F8X16[c * 16 + i]);
            oled_set_pos(x, y + 1);
            for (uint8_t i = 0; i < 8; i++)
                oled_write_data(F8X16[c * 16 + i + 8]);
            x += 8;
        }
        j++;
    }
}

/**
 * @brief 显示数字
 */
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                oled_show_char(x + (size / 2) * t, y, ' ', size, 0);
                continue;
            }
            else
            {
                enshow = 1;
            }
        }
        oled_show_char(x + (size / 2) * t, y, temp + '0', size, 0);
    }
}

/**
 * @brief 求幂
 */
static uint32_t oled_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--) result *= m;
    return result;
}

/**
 * @brief 显示汉字
 * @param index: 字库索引
 * @param mode: 0正常, 1反显
 */
void oled_show_chinese(uint8_t x, uint8_t y, uint8_t index, uint8_t mode)
{
    uint8_t wm = 0;
    uint32_t addr = 32 * index;

    oled_set_pos(x, y);
    for (wm = 0; wm < 16; wm++)
    {
        if (mode == 1)
            oled_write_data(~F16x16[addr]);
        else
            oled_write_data(F16x16[addr]);
        addr++;
    }
    oled_set_pos(x, y + 1);
    for (wm = 0; wm < 16; wm++)
    {
        if (mode == 1)
            oled_write_data(~F16x16[addr]);
        else
            oled_write_data(F16x16[addr]);
        addr++;
    }
}

/**
 * @brief 显示摄氏度符号
 */
void oled_show_centigrade(uint8_t x, uint8_t y)
{
    uint8_t wm = 0;
    const uint8_t buf[] = {
        0x10, 0x28, 0x10, 0xC0, 0x20, 0x10, 0x10, 0x10, 0x20, 0x70, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x07, 0x08, 0x10, 0x10, 0x10, 0x10, 0x08, 0x04, 0x00
    };

    oled_set_pos(x, y);
    for (wm = 0; wm < 12; wm++)
    {
        oled_write_data(buf[wm]);
    }
    oled_set_pos(x, y + 1);
    for (wm = 0; wm < 12; wm++)
    {
        oled_write_data(buf[wm + 12]);
    }
}

/**
 * @brief 显示BMP图片
 */
void oled_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t *bmp)
{
    uint32_t j = 0;
    uint8_t x, y;

    if (y1 % 8 == 0)
        y = y1 / 8;
    else
        y = y1 / 8 + 1;

    for (y = y0; y < y1; y++)
    {
        oled_set_pos(x0, y);
        for (x = x0; x < x1; x++)
        {
            oled_write_data(bmp[j++]);
        }
    }
}

/* 注册初始化函数 */
INIT_DEVICE_EXPORT(oled_init);
