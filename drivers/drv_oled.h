/*
 * OLED I2C驱动 - SSD1306 128x64
 * 基于RT-Thread I2C框架
 */
#ifndef __DRV_OLED_H__
#define __DRV_OLED_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/* OLED I2C地址 */
#define OLED_I2C_ADDR       0x3C    /* 7位地址 */
#define OLED_I2C_BUS_NAME   "i2c1"

/* OLED尺寸 */
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_PAGES          8

/* 函数声明 */
int oled_init(void);
void oled_clear(void);
void oled_fill(uint8_t data);
void oled_on(void);
void oled_off(void);
void oled_set_pos(uint8_t x, uint8_t y);
void oled_show_char(uint8_t x, uint8_t y, char chr, uint8_t size, uint8_t mode);
void oled_show_string(uint8_t x, uint8_t y, const char *str, uint8_t size);
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void oled_show_chinese(uint8_t x, uint8_t y, uint8_t index, uint8_t mode);
void oled_show_centigrade(uint8_t x, uint8_t y);
void oled_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t *bmp);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_OLED_H__ */
