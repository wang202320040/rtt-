/*
 * 智能手环 - ART-Pi II 板级配置
 * 基于STM32H750XBH6 + RT-Thread
 */
#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtthread.h>
#include <stm32h7xx.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 时钟配置 ==================== */
#define BSP_CLOCK_SOURCE                  ("HSE")
#define BSP_CLOCK_SOURCE_FREQ_MHZ         ((int32_t)25)
#define BSP_CLOCK_SYSTEM_FREQ_MHZ         ((int32_t)480)

/* ==================== GPIO引脚定义 ==================== */

/* I2C1 - 用于OLED和MAX30102 */
#define BSP_I2C1_SCL_PIN                  GET_PIN(B, 6)
#define BSP_I2C1_SDA_PIN                  GET_PIN(B, 7)

/* I2C2 - 用于ADXL345 */
#define BSP_I2C2_SCL_PIN                  GET_PIN(B, 10)
#define BSP_I2C2_SDA_PIN                  GET_PIN(B, 11)

/* DS18B20 单总线 */
#define BSP_DS18B20_PIN                   GET_PIN(A, 11)

/* DS1302 RTC */
#define BSP_DS1302_CLK_PIN                GET_PIN(C, 13)
#define BSP_DS1302_DAT_PIN                GET_PIN(C, 14)
#define BSP_DS1302_RST_PIN                GET_PIN(C, 15)

/* 按键 KEY0-KEY4 */
#define BSP_KEY0_PIN                      GET_PIN(B, 12)
#define BSP_KEY1_PIN                      GET_PIN(B, 13)
#define BSP_KEY2_PIN                      GET_PIN(B, 14)
#define BSP_KEY3_PIN                      GET_PIN(B, 15)
#define BSP_KEY4_PIN                      GET_PIN(A, 8)

/* 蜂鸣器 */
#define BSP_BEEP_PIN                      GET_PIN(B, 9)

/* LED指示灯 */
#define BSP_LED_PIN                       GET_PIN(I, 8)

/* 串口 UART1 */
#define BSP_UART1_TX_PIN                  GET_PIN(A, 9)
#define BSP_UART1_RX_PIN                  GET_PIN(A, 10)

/* ==================== I2C设备地址 ==================== */
#define OLED_I2C_ADDR                     0x3C    /* 7位地址 */
#define MAX30102_I2C_ADDR                 0x57    /* 7位地址 */
#define ADXL345_I2C_ADDR                  0x53    /* 7位地址 ALT接地 */

/* ==================== 功能使能 ==================== */
#define BSP_USING_I2C1
#define BSP_USING_I2C2
#define BSP_USING_UART1
#define BSP_USING_ONCHIP_RTC

/* ==================== 内存配置 ==================== */
#define STM32_SRAM_SIZE                   (512)
#define STM32_SRAM_END                    (0x24000000 + STM32_SRAM_SIZE * 1024)

#ifdef __RT_THREAD_H__
extern int  rt_hw_usart_init(void);
extern void rt_hw_board_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */
