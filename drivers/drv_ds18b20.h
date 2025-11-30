/*
 * DS18B20温度传感器驱动
 * 单总线协议
 */
#ifndef __DRV_DS18B20_H__
#define __DRV_DS18B20_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO引脚定义 */
#define DS18B20_PIN     GET_PIN(A, 11)

/* 函数声明 */
int ds18b20_init(void);
int16_t ds18b20_get_temp(void);
void ds18b20_start(void);
void ds18b20_write_byte(uint8_t dat);
uint8_t ds18b20_read_byte(void);
uint8_t ds18b20_read_bit(void);
uint8_t ds18b20_check(void);
void ds18b20_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_DS18B20_H__ */
