/*
 * ADXL345三轴加速度传感器驱动 - 用于计步
 */
#include "drv_adxl345.h"

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

/**
 * @brief 写寄存器
 */
void adxl345_write_reg(uint8_t addr, uint8_t val)
{
    struct rt_i2c_msg msgs;
    uint8_t buf[2];

    if (i2c_bus == RT_NULL) return;

    buf[0] = addr;
    buf[1] = val;

    msgs.addr  = ADXL345_I2C_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf   = buf;
    msgs.len   = 2;

    rt_i2c_transfer(i2c_bus, &msgs, 1);
}

/**
 * @brief 读寄存器
 */
uint8_t adxl345_read_reg(uint8_t addr)
{
    struct rt_i2c_msg msgs[2];
    uint8_t data = 0;

    if (i2c_bus == RT_NULL) return 0;

    msgs[0].addr  = ADXL345_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &addr;
    msgs[0].len   = 1;

    msgs[1].addr  = ADXL345_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = &data;
    msgs[1].len   = 1;

    rt_i2c_transfer(i2c_bus, msgs, 2);
    return data;
}

/**
 * @brief 读取加速度数据
 */
void adxl345_read_data(int16_t *x, int16_t *y, int16_t *z)
{
    struct rt_i2c_msg msgs[2];
    uint8_t reg = ADXL345_DATAX0;
    uint8_t buf[6];

    if (i2c_bus == RT_NULL) return;

    msgs[0].addr  = ADXL345_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = ADXL345_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = buf;
    msgs[1].len   = 6;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) == 2)
    {
        *x = (int16_t)((buf[1] << 8) | buf[0]);
        *y = (int16_t)((buf[3] << 8) | buf[2]);
        *z = (int16_t)((buf[5] << 8) | buf[4]);
    }
}

/**
 * @brief 读取平均加速度数据
 */
void adxl345_read_average(float *x, float *y, float *z, uint8_t times)
{
    int16_t raw_x, raw_y, raw_z;
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    uint8_t i;

    for (i = 0; i < times; i++)
    {
        adxl345_read_data(&raw_x, &raw_y, &raw_z);
        sum_x += raw_x;
        sum_y += raw_y;
        sum_z += raw_z;
    }

    /* 转换为g值 (4mg/LSB) */
    *x = (float)sum_x / times * 0.004f;
    *y = (float)sum_y / times * 0.004f;
    *z = (float)sum_z / times * 0.004f;
}

/**
 * @brief 初始化ADXL345
 */
int adxl345_init(void)
{
    uint8_t id;

    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(ADXL345_I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("ADXL345: Can't find %s device!\n", ADXL345_I2C_BUS_NAME);
        return -RT_ERROR;
    }

    /* 读取设备ID */
    id = adxl345_read_reg(ADXL345_DEVID);
    if (id != 0xE5)
    {
        rt_kprintf("ADXL345: Device ID error! (0x%02X)\n", id);
        return -RT_ERROR;
    }

    /* 配置传感器 */
    adxl345_write_reg(ADXL345_DATA_FORMAT, 0x0B);  /* 全分辨率, 16g量程 */
    adxl345_write_reg(ADXL345_BW_RATE, 0x0A);      /* 100Hz输出 */
    adxl345_write_reg(ADXL345_POWER_CTL, 0x08);    /* 测量模式 */
    adxl345_write_reg(ADXL345_OFSX, 0x00);
    adxl345_write_reg(ADXL345_OFSY, 0x00);
    adxl345_write_reg(ADXL345_OFSZ, 0x00);

    rt_kprintf("ADXL345: Initialized successfully\n");
    return RT_EOK;
}

INIT_DEVICE_EXPORT(adxl345_init);
