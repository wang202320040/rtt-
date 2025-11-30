/*
 * DS1302 RTC实时时钟驱动
 */
#include "drv_ds1302.h"
#include "board.h"

extern void delay_us(uint32_t us);

/* 全局日期变量 */
ds1302_date_t sys_date = {0, 0, 12, 1, 1, 1, 24};

#define BCD_TO_DEC(x)    ((x / 16) * 10 + (x % 16))
#define DEC_TO_BCD(x)    ((x / 10) * 16 + (x % 10))

static void ds1302_clk_high(void) { rt_pin_write(DS1302_CLK_PIN, PIN_HIGH); }
static void ds1302_clk_low(void)  { rt_pin_write(DS1302_CLK_PIN, PIN_LOW); }
static void ds1302_dat_high(void) { rt_pin_write(DS1302_DAT_PIN, PIN_HIGH); }
static void ds1302_dat_low(void)  { rt_pin_write(DS1302_DAT_PIN, PIN_LOW); }
static void ds1302_rst_high(void) { rt_pin_write(DS1302_RST_PIN, PIN_HIGH); }
static void ds1302_rst_low(void)  { rt_pin_write(DS1302_RST_PIN, PIN_LOW); }
static uint8_t ds1302_dat_read(void) { return rt_pin_read(DS1302_DAT_PIN); }

static void ds1302_dat_out(void) { rt_pin_mode(DS1302_DAT_PIN, PIN_MODE_OUTPUT); }
static void ds1302_dat_in(void)  { rt_pin_mode(DS1302_DAT_PIN, PIN_MODE_INPUT); }

/**
 * @brief 写入一个字节
 */
void ds1302_write_byte(uint8_t addr, uint8_t data)
{
    uint8_t i;

    ds1302_rst_low();
    delay_us(2);
    ds1302_clk_low();
    delay_us(2);
    ds1302_rst_high();
    delay_us(2);

    ds1302_dat_out();

    /* 写地址 */
    for (i = 0; i < 8; i++)
    {
        ds1302_clk_low();
        if (addr & 0x01)
            ds1302_dat_high();
        else
            ds1302_dat_low();
        delay_us(2);
        ds1302_clk_high();
        delay_us(2);
        addr >>= 1;
    }

    /* 写数据 */
    for (i = 0; i < 8; i++)
    {
        ds1302_clk_low();
        if (data & 0x01)
            ds1302_dat_high();
        else
            ds1302_dat_low();
        delay_us(2);
        ds1302_clk_high();
        delay_us(2);
        data >>= 1;
    }

    ds1302_rst_low();
    delay_us(2);
}

/**
 * @brief 读取一个字节
 */
uint8_t ds1302_read_byte(uint8_t addr)
{
    uint8_t i, data = 0;

    ds1302_rst_low();
    delay_us(2);
    ds1302_clk_low();
    delay_us(2);
    ds1302_rst_high();
    delay_us(2);

    ds1302_dat_out();

    /* 写地址 */
    for (i = 0; i < 8; i++)
    {
        ds1302_clk_low();
        if (addr & 0x01)
            ds1302_dat_high();
        else
            ds1302_dat_low();
        delay_us(2);
        ds1302_clk_high();
        delay_us(2);
        addr >>= 1;
    }

    ds1302_dat_in();

    /* 读数据 */
    for (i = 0; i < 8; i++)
    {
        ds1302_clk_low();
        delay_us(2);
        data >>= 1;
        if (ds1302_dat_read())
            data |= 0x80;
        ds1302_clk_high();
        delay_us(2);
    }

    ds1302_rst_low();
    delay_us(2);

    return data;
}

/**
 * @brief 设置日期时间
 */
int ds1302_set_date(const ds1302_date_t *date)
{
    if (date == RT_NULL) return -RT_ERROR;

    ds1302_write_byte(DS1302_WRITE_CTRL, 0x00);  /* 取消写保护 */

    ds1302_write_byte(DS1302_WRITE_SEC, DEC_TO_BCD(date->sec));
    ds1302_write_byte(DS1302_WRITE_MIN, DEC_TO_BCD(date->min));
    ds1302_write_byte(DS1302_WRITE_HOUR, DEC_TO_BCD(date->hour));
    ds1302_write_byte(DS1302_WRITE_DAY, DEC_TO_BCD(date->day));
    ds1302_write_byte(DS1302_WRITE_MON, DEC_TO_BCD(date->mon));
    ds1302_write_byte(DS1302_WRITE_WEEK, DEC_TO_BCD(date->week));
    ds1302_write_byte(DS1302_WRITE_YEAR, DEC_TO_BCD(date->year % 100));

    ds1302_write_byte(DS1302_WRITE_CTRL, 0x80);  /* 写保护 */

    return RT_EOK;
}

/**
 * @brief 读取日期时间
 */
int ds1302_read_date(ds1302_date_t *date)
{
    if (date == RT_NULL) return -RT_ERROR;

    date->sec = BCD_TO_DEC(ds1302_read_byte(DS1302_READ_SEC) & 0x7F);
    date->min = BCD_TO_DEC(ds1302_read_byte(DS1302_READ_MIN) & 0x7F);
    date->hour = BCD_TO_DEC(ds1302_read_byte(DS1302_READ_HOUR) & 0x3F);
    date->day = BCD_TO_DEC(ds1302_read_byte(DS1302_READ_DAY) & 0x3F);
    date->mon = BCD_TO_DEC(ds1302_read_byte(DS1302_READ_MON) & 0x1F);
    date->week = BCD_TO_DEC(ds1302_read_byte(DS1302_READ_WEEK) & 0x07);
    date->year = 2000 + BCD_TO_DEC(ds1302_read_byte(DS1302_READ_YEAR));

    return RT_EOK;
}

/**
 * @brief 初始化DS1302
 */
int ds1302_init(const ds1302_date_t *date)
{
    rt_pin_mode(DS1302_CLK_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DS1302_DAT_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DS1302_RST_PIN, PIN_MODE_OUTPUT);

    ds1302_rst_low();
    ds1302_clk_low();

    ds1302_write_byte(DS1302_WRITE_CTRL, 0x00);  /* 取消写保护 */
    ds1302_write_byte(DS1302_WRITE_SEC, 0x00);   /* 启动时钟 */

    if (date != RT_NULL)
    {
        ds1302_set_date(date);
    }

    rt_kprintf("DS1302: Initialized successfully\n");
    return RT_EOK;
}
