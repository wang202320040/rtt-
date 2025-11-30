/*
 * DS1302 RTC实时时钟驱动
 */
#ifndef __DRV_DS1302_H__
#define __DRV_DS1302_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO引脚定义 */
#define DS1302_CLK_PIN    GET_PIN(C, 13)
#define DS1302_DAT_PIN    GET_PIN(C, 14)
#define DS1302_RST_PIN    GET_PIN(C, 15)

/* 寄存器地址 */
#define DS1302_WRITE_SEC     0x80
#define DS1302_WRITE_MIN     0x82
#define DS1302_WRITE_HOUR    0x84
#define DS1302_WRITE_DAY     0x86
#define DS1302_WRITE_MON     0x88
#define DS1302_WRITE_WEEK    0x8A
#define DS1302_WRITE_YEAR    0x8C
#define DS1302_READ_SEC      0x81
#define DS1302_READ_MIN      0x83
#define DS1302_READ_HOUR     0x85
#define DS1302_READ_DAY      0x87
#define DS1302_READ_MON      0x89
#define DS1302_READ_WEEK     0x8B
#define DS1302_READ_YEAR     0x8D
#define DS1302_WRITE_CTRL    0x8E
#define DS1302_READ_CTRL     0x8F

/* 日期时间结构 */
typedef struct {
    volatile uint8_t sec;
    volatile uint8_t min;
    volatile uint8_t hour;
    volatile uint8_t day;
    volatile uint8_t mon;
    volatile uint8_t week;
    volatile uint16_t year;
} ds1302_date_t;

/* 全局日期变量 */
extern ds1302_date_t sys_date;

/* 函数声明 */
int ds1302_init(const ds1302_date_t *date);
int ds1302_set_date(const ds1302_date_t *date);
int ds1302_read_date(ds1302_date_t *date);
void ds1302_write_byte(uint8_t addr, uint8_t data);
uint8_t ds1302_read_byte(uint8_t addr);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_DS1302_H__ */
