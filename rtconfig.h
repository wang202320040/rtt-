/*
 * 智能手环 - RT-Thread配置文件
 * 针对ART-Pi II (STM32H750)
 */
#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* RT-Thread 基础配置 */
#define RT_NAME_MAX                       8
#define RT_ALIGN_SIZE                     4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX            32
#define RT_TICK_PER_SECOND                1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE            4
#define IDLE_THREAD_STACK_SIZE            256

/* 调试配置 */
#define RT_DEBUG

/* 线程间同步与通信 */
#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* 内存管理 */
#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* 内核设备对象 */
#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE                256
#define RT_CONSOLE_DEVICE_NAME            "uart1"

/* 自动初始化 */
#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE         4096
#define RT_MAIN_THREAD_PRIORITY           10

/* 软件定时器 */
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO              4
#define RT_TIMER_THREAD_STACK_SIZE        512

/* Finsh Shell */
#define RT_USING_FINSH
#define FINSH_THREAD_NAME                 "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES               5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY             20
#define FINSH_THREAD_STACK_SIZE           4096
#define FINSH_CMD_SIZE                    80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX                     10

/* 设备驱动框架 */
#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ                     512
#define RT_USING_SERIAL
#define RT_SERIAL_USING_DMA
#define RT_SERIAL_RB_BUFSZ                256

/* I2C驱动 */
#define RT_USING_I2C
#define RT_USING_I2C_BITOPS

/* PIN驱动 */
#define RT_USING_PIN

/* RTC驱动 */
#define RT_USING_RTC

/* 软件包 */
#define PKG_USING_FAL

/* 板级配置 */
#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32H7
#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
#define BSP_UART1_TX_PIN                  "PA9"
#define BSP_UART1_RX_PIN                  "PA10"

/* I2C配置 */
#define BSP_USING_I2C1
#define BSP_I2C1_SCL_PIN                  GET_PIN(B, 6)
#define BSP_I2C1_SDA_PIN                  GET_PIN(B, 7)

#define BSP_USING_I2C2
#define BSP_I2C2_SCL_PIN                  GET_PIN(B, 10)
#define BSP_I2C2_SDA_PIN                  GET_PIN(B, 11)

/* 堆配置 */
#define HEAP_BEGIN                        ((void *)&__bss_end)
#define HEAP_END                          ((void *)(0x24000000 + 512 * 1024))
extern int __bss_end;

#endif /* RT_CONFIG_H__ */
