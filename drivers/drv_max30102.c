/*
 * MAX30102心率血氧传感器驱动
 * 基于RT-Thread I2C框架
 */
#include "drv_max30102.h"
#include "algorithm.h"

/* I2C设备句柄 */
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

/* 数据缓冲区 */
static uint32_t aun_ir_buffer[150];
static uint32_t aun_red_buffer[150];
static int32_t n_ir_buffer_length = 150;

/* 心率血氧计算结果 */
static int32_t n_spo2;
static int8_t ch_spo2_valid;
static int32_t n_heart_rate;
static int8_t ch_hr_valid;

/* 滤波缓冲区 */
static int32_t hr_buf[16];
static int32_t spo2_buf[16];
static int32_t hr_avg = 0;
static int32_t spo2_avg = 0;
static int32_t hr_buff_filled = 0;
static int32_t spo2_buff_filled = 0;

/* 信号范围 */
static uint32_t un_min, un_max, un_prev_data;

/**
 * @brief 写寄存器
 */
bool max30102_write_reg(uint8_t reg, uint8_t data)
{
    struct rt_i2c_msg msgs;
    uint8_t buf[2];

    if (i2c_bus == RT_NULL)
        return false;

    buf[0] = reg;
    buf[1] = data;

    msgs.addr  = MAX30102_I2C_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf   = buf;
    msgs.len   = 2;

    if (rt_i2c_transfer(i2c_bus, &msgs, 1) == 1)
        return true;
    else
        return false;
}

/**
 * @brief 读寄存器
 */
bool max30102_read_reg(uint8_t reg, uint8_t *data)
{
    struct rt_i2c_msg msgs[2];

    if (i2c_bus == RT_NULL || data == RT_NULL)
        return false;

    msgs[0].addr  = MAX30102_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = MAX30102_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = data;
    msgs[1].len   = 1;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) == 2)
        return true;
    else
        return false;
}

/**
 * @brief 读取FIFO数据
 */
bool max30102_read_fifo(uint32_t *red_led, uint32_t *ir_led)
{
    struct rt_i2c_msg msgs[2];
    uint8_t reg = REG_FIFO_DATA;
    uint8_t buf[6];
    uint8_t temp;

    if (i2c_bus == RT_NULL || red_led == RT_NULL || ir_led == RT_NULL)
        return false;

    *red_led = 0;
    *ir_led = 0;

    /* 清除中断状态 */
    max30102_read_reg(REG_INTR_STATUS_1, &temp);
    max30102_read_reg(REG_INTR_STATUS_2, &temp);

    /* 读取FIFO数据 (6字节) */
    msgs[0].addr  = MAX30102_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = MAX30102_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = buf;
    msgs[1].len   = 6;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) != 2)
        return false;

    /* 解析RED数据 (前3字节) */
    *red_led = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    *red_led &= 0x03FFFF;

    /* 解析IR数据 (后3字节) */
    *ir_led = ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 8) | buf[5];
    *ir_led &= 0x03FFFF;

    return true;
}

/**
 * @brief 复位传感器
 */
bool max30102_reset(void)
{
    return max30102_write_reg(REG_MODE_CONFIG, 0x40);
}

/**
 * @brief 初始化MAX30102
 */
int max30102_init(void)
{
    int32_t i;
    uint8_t temp;

    /* 查找I2C总线 */
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(MAX30102_I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("MAX30102: Can't find %s device!\n", MAX30102_I2C_BUS_NAME);
        return -RT_ERROR;
    }

    un_min = 0x3FFFF;
    un_max = 0;

    /* 复位传感器 */
    max30102_reset();
    rt_thread_mdelay(10);

    /* 清除中断状态 */
    max30102_read_reg(REG_INTR_STATUS_1, &temp);

    /* 配置寄存器 */
    if (!max30102_write_reg(REG_INTR_ENABLE_1, 0xC0)) return -RT_ERROR;
    if (!max30102_write_reg(REG_INTR_ENABLE_2, 0x00)) return -RT_ERROR;
    if (!max30102_write_reg(REG_FIFO_WR_PTR, 0x00)) return -RT_ERROR;
    if (!max30102_write_reg(REG_OVF_COUNTER, 0x00)) return -RT_ERROR;
    if (!max30102_write_reg(REG_FIFO_RD_PTR, 0x00)) return -RT_ERROR;
    if (!max30102_write_reg(REG_FIFO_CONFIG, 0x6F)) return -RT_ERROR;  /* sample avg=8, fifo rollover=false */
    if (!max30102_write_reg(REG_MODE_CONFIG, 0x03)) return -RT_ERROR;  /* SpO2 mode */
    if (!max30102_write_reg(REG_SPO2_CONFIG, 0x2F)) return -RT_ERROR;  /* ADC range=4096nA, 400Hz, 411uS */
    if (!max30102_write_reg(REG_LED1_PA, 0x17)) return -RT_ERROR;      /* ~4.5mA for LED1 */
    if (!max30102_write_reg(REG_LED2_PA, 0x17)) return -RT_ERROR;      /* ~4.5mA for LED2 */
    if (!max30102_write_reg(REG_PILOT_PA, 0x7F)) return -RT_ERROR;     /* ~25mA for Pilot LED */

    /* 读取初始样本 */
    for (i = 0; i < n_ir_buffer_length; i++)
    {
        max30102_read_fifo(&aun_red_buffer[i], &aun_ir_buffer[i]);
        if (un_min > aun_red_buffer[i])
            un_min = aun_red_buffer[i];
        if (un_max < aun_red_buffer[i])
            un_max = aun_red_buffer[i];
    }
    un_prev_data = aun_red_buffer[i - 1];

    /* 初始计算 */
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer,
                                           &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

    rt_kprintf("MAX30102: Initialized successfully\n");
    return RT_EOK;
}

/**
 * @brief 读取心率血氧数据
 */
void max30102_read_data(max30102_data_t *data)
{
    int32_t i;
    static uint8_t count = 0;
    int32_t hr_sum, spo2_sum;
    int32_t hr_valid_cnt = 0;
    int32_t spo2_valid_cnt = 0;
    static int32_t hr_timeout = 0;
    static int32_t spo2_timeout = 0;

    /* 移动缓冲区数据 */
    un_min = 0x3FFFF;
    un_max = 0;
    for (i = 50; i < 150; i++)
    {
        aun_red_buffer[i - 50] = aun_red_buffer[i];
        aun_ir_buffer[i - 50] = aun_ir_buffer[i];
        if (un_min > aun_red_buffer[i])
            un_min = aun_red_buffer[i];
        if (un_max < aun_red_buffer[i])
            un_max = aun_red_buffer[i];
    }

    /* 读取新的50个样本 */
    for (i = 100; i < 150; i++)
    {
        un_prev_data = aun_red_buffer[i - 1];
        max30102_read_fifo(&aun_red_buffer[i], &aun_ir_buffer[i]);
    }

    /* 计算心率和血氧 */
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer,
                                           &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

    /* 滤波处理 (每8次更新一次) */
    if (++count > 8)
    {
        count = 0;

        /* 心率滤波 */
        if ((ch_hr_valid == 1) && (n_heart_rate < 150) && (n_heart_rate > 60))
        {
            hr_timeout = 0;

            /* 移位新样本到缓冲区 */
            for (i = 0; i < 15; i++)
                hr_buf[i] = hr_buf[i + 1];
            hr_buf[15] = n_heart_rate;

            if (hr_buff_filled < 16)
                hr_buff_filled++;

            /* 滑动平均 */
            hr_sum = 0;
            if (hr_buff_filled < 2)
            {
                hr_avg = 0;
            }
            else if (hr_buff_filled < 4)
            {
                for (i = 14; i < 16; i++)
                    hr_sum += hr_buf[i];
                hr_avg = hr_sum >> 1;
            }
            else if (hr_buff_filled < 8)
            {
                for (i = 12; i < 16; i++)
                    hr_sum += hr_buf[i];
                hr_avg = hr_sum >> 2;
            }
            else if (hr_buff_filled < 16)
            {
                for (i = 8; i < 16; i++)
                    hr_sum += hr_buf[i];
                hr_avg = hr_sum >> 3;
            }
            else
            {
                for (i = 0; i < 16; i++)
                    hr_sum += hr_buf[i];
                hr_avg = hr_sum >> 4;
            }
        }
        else
        {
            if (hr_timeout >= 2)
            {
                hr_avg = 0;
                hr_buff_filled = 0;
            }
            else
            {
                hr_timeout++;
            }
        }

        /* 血氧滤波 */
        if ((ch_spo2_valid == 1) && (n_spo2 > 80))
        {
            spo2_timeout = 0;

            for (i = 0; i < 15; i++)
                spo2_buf[i] = spo2_buf[i + 1];
            spo2_buf[15] = n_spo2;

            if (spo2_buff_filled < 16)
                spo2_buff_filled++;

            spo2_sum = 0;
            if (spo2_buff_filled < 2)
            {
                spo2_avg = 0;
            }
            else if (spo2_buff_filled < 4)
            {
                for (i = 14; i < 16; i++)
                    spo2_sum += spo2_buf[i];
                spo2_avg = spo2_sum >> 1;
            }
            else if (spo2_buff_filled < 8)
            {
                for (i = 12; i < 16; i++)
                    spo2_sum += spo2_buf[i];
                spo2_avg = spo2_sum >> 2;
            }
            else if (spo2_buff_filled < 16)
            {
                for (i = 8; i < 16; i++)
                    spo2_sum += spo2_buf[i];
                spo2_avg = spo2_sum >> 3;
            }
            else
            {
                for (i = 0; i < 16; i++)
                    spo2_sum += spo2_buf[i];
                spo2_avg = spo2_sum >> 4;
            }
        }
        else
        {
            if (spo2_timeout >= 2)
            {
                spo2_avg = 0;
                spo2_buff_filled = 0;
            }
            else
            {
                spo2_timeout++;
            }
        }
    }

    /* 返回结果 */
    if (data != RT_NULL)
    {
        data->heart_rate = hr_avg;
        data->spo2 = spo2_avg;
        data->hr_valid = (hr_avg > 0) ? 1 : 0;
        data->spo2_valid = (spo2_avg > 0) ? 1 : 0;
    }
}

/**
 * @brief 获取心率值
 */
int32_t max30102_get_heart_rate(void)
{
    return hr_avg;
}

/**
 * @brief 获取血氧值
 */
int32_t max30102_get_spo2(void)
{
    return spo2_avg;
}

/* 注册初始化函数 */
INIT_DEVICE_EXPORT(max30102_init);
