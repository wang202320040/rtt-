/*
 * MAX30102心率血氧传感器驱动
 * 基于RT-Thread I2C框架
 */
#ifndef __DRV_MAX30102_H__
#define __DRV_MAX30102_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* I2C配置 */
#define MAX30102_I2C_BUS_NAME   "i2c1"
#define MAX30102_I2C_ADDR       0x57    /* 7位地址 */

/* 寄存器地址 */
#define REG_INTR_STATUS_1       0x00
#define REG_INTR_STATUS_2       0x01
#define REG_INTR_ENABLE_1       0x02
#define REG_INTR_ENABLE_2       0x03
#define REG_FIFO_WR_PTR         0x04
#define REG_OVF_COUNTER         0x05
#define REG_FIFO_RD_PTR         0x06
#define REG_FIFO_DATA           0x07
#define REG_FIFO_CONFIG         0x08
#define REG_MODE_CONFIG         0x09
#define REG_SPO2_CONFIG         0x0A
#define REG_LED1_PA             0x0C
#define REG_LED2_PA             0x0D
#define REG_PILOT_PA            0x10
#define REG_MULTI_LED_CTRL1     0x11
#define REG_MULTI_LED_CTRL2     0x12
#define REG_TEMP_INTR           0x1F
#define REG_TEMP_FRAC           0x20
#define REG_TEMP_CONFIG         0x21
#define REG_PROX_INT_THRESH     0x30
#define REG_REV_ID              0xFE
#define REG_PART_ID             0xFF

/* 心率血氧数据结构 */
typedef struct {
    int32_t heart_rate;     /* 心率值 */
    int32_t spo2;           /* 血氧浓度 */
    uint8_t hr_valid;       /* 心率有效标志 */
    uint8_t spo2_valid;     /* 血氧有效标志 */
} max30102_data_t;

/* 函数声明 */
int max30102_init(void);
bool max30102_write_reg(uint8_t reg, uint8_t data);
bool max30102_read_reg(uint8_t reg, uint8_t *data);
bool max30102_read_fifo(uint32_t *red_led, uint32_t *ir_led);
bool max30102_reset(void);
void max30102_read_data(max30102_data_t *data);
int32_t max30102_get_heart_rate(void);
int32_t max30102_get_spo2(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_MAX30102_H__ */
