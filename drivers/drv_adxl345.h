/*
 * ADXL345三轴加速度传感器驱动 - 用于计步
 * 基于RT-Thread I2C框架
 */
#ifndef __DRV_ADXL345_H__
#define __DRV_ADXL345_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/* I2C配置 */
#define ADXL345_I2C_BUS_NAME   "i2c2"
#define ADXL345_I2C_ADDR       0x53    /* 7位地址 ALT接地 */

/* 寄存器地址 */
#define ADXL345_DEVID          0x00
#define ADXL345_OFSX           0x1E
#define ADXL345_OFSY           0x1F
#define ADXL345_OFSZ           0x20
#define ADXL345_BW_RATE        0x2C
#define ADXL345_POWER_CTL      0x2D
#define ADXL345_DATA_FORMAT    0x31
#define ADXL345_DATAX0         0x32
#define ADXL345_DATAX1         0x33
#define ADXL345_DATAY0         0x34
#define ADXL345_DATAY1         0x35
#define ADXL345_DATAZ0         0x36
#define ADXL345_DATAZ1         0x37

/* 数据结构 */
typedef struct {
    float x;
    float y;
    float z;
} adxl345_data_t;

/* 函数声明 */
int adxl345_init(void);
uint8_t adxl345_read_reg(uint8_t addr);
void adxl345_write_reg(uint8_t addr, uint8_t val);
void adxl345_read_data(int16_t *x, int16_t *y, int16_t *z);
void adxl345_read_average(float *x, float *y, float *z, uint8_t times);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ADXL345_H__ */
