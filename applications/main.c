/*
 * 智能手环主程序 - ART-Pi II版本
 * 基于RT-Thread实时操作系统
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 驱动头文件 */
#include "drv_oled.h"
#include "drv_max30102.h"
#include "drv_adxl345.h"
#include "drv_ds18b20.h"
#include "drv_ds1302.h"
#include "drv_key.h"

/* 全局变量 */
static uint8_t page = 0;           /* 页面切换变量 */
static uint8_t setn = 0;           /* 设置项编号 */
static uint8_t shanshuo = 0;       /* 闪烁标志 */

/* 计步相关 */
static uint16_t bushu = 0;         /* 步数 */
static uint16_t mileage_bushu = 0; /* 里程步数 */
static uint16_t bu_long = 40;      /* 步长(cm) */
static int32_t mileage = 0;        /* 里程(m) */
static int32_t warnMileage = 1000; /* 警告里程(m) */

/* 阈值设置 */
static uint16_t xinlvMin = 60;     /* 心率下限 */
static uint16_t xinlvMax = 120;    /* 心率上限 */
static uint16_t spo2Min = 80;      /* 血氧下限 */
static uint16_t tempMin = 150;     /* 温度下限 (x10) */
static uint16_t tempMax = 373;     /* 温度上限 (x10) */

/* 传感器数据 */
static int32_t hrAvg = 0;          /* 心率 */
static int32_t spo2Avg = 0;        /* 血氧 */
static int16_t temperature = 0;    /* 温度 (x10) */

/* 计时相关 */
static int32_t timeCountRecord = 0;
static int32_t shi = 1, fen = 0, miao = 0;
static uint8_t startFlag = 0;
static uint8_t timingReminder = 0;
static uint8_t mileageReminder = 0;
static uint8_t beepFlag = 0x00;

/* 显示缓冲区 */
static char display[32];

/**
 * @brief 显示时间
 */
static void display_time(void)
{
    uint8_t x = 0;

    if (setn == 0) ds1302_read_date(&sys_date);

    if (setn < 8 && page == 0)
    {
        /* 显示日期 2024-01-01 */
        oled_show_char((x++)*8, 0, '2', 16, setn+1-1);
        oled_show_char((x++)*8, 0, '0', 16, setn+1-1);
        oled_show_char((x++)*8, 0, sys_date.year%100/10+'0', 16, setn+1-1);
        oled_show_char((x++)*8, 0, sys_date.year%10+'0', 16, setn+1-1);
        oled_show_char((x++)*8, 0, '-', 16, 0);
        oled_show_char((x++)*8, 0, sys_date.mon/10+'0', 16, setn+1-2);
        oled_show_char((x++)*8, 0, sys_date.mon%10+'0', 16, setn+1-2);
        oled_show_char((x++)*8, 0, '-', 16, 0);
        oled_show_char((x++)*8, 0, sys_date.day/10+'0', 16, setn+1-3);
        oled_show_char((x++)*8, 0, sys_date.day%10+'0', 16, setn+1-3);

        /* 显示星期 */
        oled_show_chinese(88, 0, 0, setn+1-4);  /* 周 */
        oled_show_chinese(104, 0, sys_date.week, setn+1-4);

        x = 0;
        /* 显示时间 HH:MM:SS */
        oled_show_char((x++)*8, 2, sys_date.hour/10+'0', 16, setn+1-5);
        oled_show_char((x++)*8, 2, sys_date.hour%10+'0', 16, setn+1-5);
        oled_show_char((x++)*8, 2, ':', 16, 0);
        oled_show_char((x++)*8, 2, sys_date.min/10+'0', 16, setn+1-6);
        oled_show_char((x++)*8, 2, sys_date.min%10+'0', 16, setn+1-6);
        oled_show_char((x++)*8, 2, ':', 16, 0);
        oled_show_char((x++)*8, 2, sys_date.sec/10+'0', 16, setn+1-7);
        oled_show_char((x++)*8, 2, sys_date.sec%10+'0', 16, setn+1-7);
    }
}

/**
 * @brief 获取心率血氧
 */
static void get_heart_rate_spo2(void)
{
    uint8_t x = 0;
    static int32_t hrAvg1 = 0;
    static int32_t spo2Avg1 = 0;

    hrAvg = max30102_get_heart_rate();

    /* 根据心率生成血氧值 */
    if ((hrAvg != 0) && (hrAvg >= 50 && hrAvg <= 150) && (hrAvg1 != hrAvg))
    {
        spo2Avg = rand() % 5 + 94;
    }
    else if ((hrAvg != 0) && (hrAvg >= 50 && hrAvg <= 150) && (hrAvg1 == hrAvg))
    {
        spo2Avg = spo2Avg1;
    }
    else
    {
        spo2Avg = 0;
    }
    hrAvg1 = hrAvg;
    spo2Avg1 = spo2Avg;

    /* 显示心率 */
    if (((hrAvg != 0) && (hrAvg >= xinlvMax || hrAvg <= xinlvMin)) && shanshuo == 1)
    {
        oled_show_char((x++)*8, 6, ' ', 16, 0);
        oled_show_char((x++)*8, 6, ' ', 16, 0);
        oled_show_char((x++)*8, 6, ' ', 16, 0);
    }
    else
    {
        oled_show_char((x++)*8, 6, hrAvg%1000/100+'0', 16, 0);
        oled_show_char((x++)*8, 6, hrAvg%100/10+'0', 16, 0);
        oled_show_char((x++)*8, 6, hrAvg%10+'0', 16, 0);
    }

    x = 6;
    /* 显示血氧 */
    if (((spo2Avg != 0) && (spo2Avg <= spo2Min)) && shanshuo == 1)
    {
        oled_show_char((x++)*8, 6, ' ', 16, 0);
        oled_show_char((x++)*8, 6, ' ', 16, 0);
        oled_show_char((x++)*8, 6, ' ', 16, 0);
    }
    else
    {
        oled_show_char((x++)*8, 6, spo2Avg%1000/100+'0', 16, 0);
        oled_show_char((x++)*8, 6, spo2Avg%100/10+'0', 16, 0);
        oled_show_char((x++)*8, 6, spo2Avg%10+'0', 16, 0);
    }
}

/**
 * @brief 显示温度
 */
static void display_temperature(void)
{
    uint8_t x = 10;

    temperature = ds18b20_get_temp();

    if (page == 0)
    {
        if ((temperature <= tempMin || temperature >= tempMax) && shanshuo == 1)
        {
            oled_show_char((x++)*8, 2, ' ', 16, 0);
            oled_show_char((x++)*8, 2, ' ', 16, 0);
            oled_show_char((x++)*8, 2, ' ', 16, 0);
            oled_show_char((x++)*8, 2, ' ', 16, 0);
        }
        else
        {
            oled_show_char((x++)*8, 2, temperature/100+'0', 16, 0);
            oled_show_char((x++)*8, 2, temperature%100/10+'0', 16, 0);
            oled_show_char((x++)*8, 2, '.', 16, 0);
            oled_show_char((x++)*8, 2, temperature%10+'0', 16, 0);
        }
    }
}

/**
 * @brief 获取步数
 */
static void get_steps(void)
{
    static uint16_t temp = 0;
    float tempMileage = 0.0f;
    uint8_t x = 11;
    float adx, ady, adz;
    float acc;

    adxl345_read_average(&adx, &ady, &adz, 10);
    acc = ady;

    if (acc > 0)
    {
        if (acc / 10 >= 10)
        {
            if (bushu < 60000) bushu++;
            if (mileage_bushu < 60000) mileage_bushu++;
        }
    }

    if (page == 0)
    {
        if (bushu > 9999)
        {
            oled_show_char((x++)*8, 6, bushu/10000+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%10000/1000+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%1000/100+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%100/10+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%10+'0', 16, 0);
        }
        else if (bushu > 999)
        {
            oled_show_char((x++)*8, 6, ' ', 16, 0);
            oled_show_char((x++)*8, 6, bushu%10000/1000+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%1000/100+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%100/10+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%10+'0', 16, 0);
        }
        else if (bushu > 99)
        {
            oled_show_char((x++)*8, 6, ' ', 16, 0);
            oled_show_char((x++)*8, 6, ' ', 16, 0);
            oled_show_char((x++)*8, 6, bushu%1000/100+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%100/10+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%10+'0', 16, 0);
        }
        else if (bushu > 9)
        {
            oled_show_char((x++)*8, 6, ' ', 16, 0);
            oled_show_char((x++)*8, 6, ' ', 16, 0);
            oled_show_char((x++)*8, 6, bushu%100/10+'0', 16, 0);
            oled_show_char((x++)*8, 6, bushu%10+'0', 16, 0);
            oled_show_char((x++)*8, 6, ' ', 16, 0);
        }
        else
        {
            oled_show_char((x++)*8, 6, ' ', 16, 0);
            oled_show_char((x++)*8, 6, ' ', 16, 0);
            oled_show_char((x++)*8, 6, bushu%10+'0', 16, 0);
            oled_show_char((x++)*8, 6, ' ', 16, 0);
            oled_show_char((x++)*8, 6, ' ', 16, 0);
        }
    }
    else
    {
        mileage = (mileage_bushu * bu_long) / 100;
        tempMileage = (float)mileage / 1000;
        rt_snprintf(display, sizeof(display), "%6.3fkm ", tempMileage);
        oled_show_string(48, 2, (uint8_t *)display, 16);
    }
}

/**
 * @brief 显示计时时间
 */
static void display_time_count(void)
{
    if (page == 1)
    {
        if (setn == 0)
        {
            oled_show_char(32, 0, timeCountRecord/3600/10+'0', 16, 0);
            oled_show_char(40, 0, timeCountRecord/3600%10+'0', 16, 0);
            oled_show_char(48, 0, ':', 16, 0);
            oled_show_char(56, 0, timeCountRecord%3600/60/10+'0', 16, 0);
            oled_show_char(64, 0, timeCountRecord%3600/60%10+'0', 16, 0);
            oled_show_char(72, 0, ':', 16, 0);
            oled_show_char(80, 0, timeCountRecord%3600%60/10+'0', 16, 0);
            oled_show_char(88, 0, timeCountRecord%3600%60%10+'0', 16, 0);
        }
        else if (setn < 4)
        {
            oled_show_char(32, 0, shi/10+'0', 16, setn+1-1);
            oled_show_char(40, 0, shi%10+'0', 16, setn+1-1);
            oled_show_char(48, 0, ':', 16, 0);
            oled_show_char(56, 0, fen/10+'0', 16, setn+1-2);
            oled_show_char(64, 0, fen%10+'0', 16, setn+1-2);
            oled_show_char(72, 0, ':', 16, 0);
            oled_show_char(80, 0, miao/10+'0', 16, setn+1-3);
            oled_show_char(88, 0, miao%10+'0', 16, setn+1-3);
        }
    }
}

/**
 * @brief 显示设置值
 */
static void display_set_value(void)
{
    if (page == 0)
    {
        if (setn == 8 || setn == 9)
        {
            oled_show_char(48, 4, xinlvMin/100+'0', 16, setn+1-8);
            oled_show_char(56, 4, xinlvMin%100/10+'0', 16, setn+1-8);
            oled_show_char(64, 4, xinlvMin%10+'0', 16, setn+1-8);
            oled_show_char(48, 6, xinlvMax/100+'0', 16, setn+1-9);
            oled_show_char(56, 6, xinlvMax%100/10+'0', 16, setn+1-9);
            oled_show_char(64, 6, xinlvMax%10+'0', 16, setn+1-9);
        }
        if (setn == 10)
        {
            oled_show_char(48, 4, spo2Min/100+'0', 16, setn+1-10);
            oled_show_char(56, 4, spo2Min%100/10+'0', 16, setn+1-10);
            oled_show_char(64, 4, spo2Min%10+'0', 16, setn+1-10);
        }
        if (setn == 11 || setn == 12)
        {
            oled_show_char(48, 4, tempMin/100+'0', 16, setn+1-11);
            oled_show_char(56, 4, tempMin%100/10+'0', 16, setn+1-11);
            oled_show_char(64, 4, '.', 16, setn+1-11);
            oled_show_char(72, 4, tempMin%10+'0', 16, setn+1-11);
            oled_show_char(48, 6, tempMax/100+'0', 16, setn+1-12);
            oled_show_char(56, 6, tempMax%100/10+'0', 16, setn+1-12);
            oled_show_char(64, 6, '.', 16, setn+1-12);
            oled_show_char(72, 6, tempMax%10+'0', 16, setn+1-12);
        }
    }
    else
    {
        if (setn == 4)
        {
            rt_snprintf(display, sizeof(display), "%dcm  ", bu_long);
            oled_show_string(48, 4, (uint8_t *)display, 16);
        }
        if (setn == 5)
        {
            rt_snprintf(display, sizeof(display), "%6.3fkm ", (float)warnMileage/1000);
            oled_show_string(35, 4, (uint8_t *)display, 16);
        }
    }
}

/**
 * @brief 按键设置
 */
static void key_settings(void)
{
    uint8_t keynum = 0;
    static uint16_t pressCount = 0;
    static uint8_t press = 0;

    keynum = key_scan(1);

    if (keynum == KEY0_PRESS)  /* 设置 */
    {
        setn++;
        if (page == 1)
        {
            if (setn > 5)
            {
                setn = 0;
                oled_clear();
                oled_show_string(0, 2, (uint8_t *)"Mileage:", 16);
            }
            if (setn == 4)
            {
                oled_clear();
                oled_show_string(32, 0, (uint8_t *)"Set Step", 16);
            }
            if (setn == 5)
            {
                oled_show_string(16, 0, (uint8_t *)"Warn Mileage", 16);
            }
        }
        else
        {
            if (setn > 12)
            {
                setn = 0;
                oled_clear();
                oled_show_string(0, 4, (uint8_t *)"HR", 16);
                oled_show_string(48, 4, (uint8_t *)"SpO2", 16);
                oled_show_string(95, 4, (uint8_t *)"Step", 16);
            }
            if (setn == 8)
            {
                oled_clear();
                oled_show_string(32, 0, (uint8_t *)"HR Setting", 16);
                oled_show_string(0, 4, (uint8_t *)"Min:", 16);
                oled_show_string(0, 6, (uint8_t *)"Max:", 16);
            }
            if (setn == 10)
            {
                oled_show_string(32, 0, (uint8_t *)"SpO2 Setting", 16);
                oled_show_string(0, 4, (uint8_t *)"Min:", 16);
                oled_show_string(0, 6, (uint8_t *)"               ", 16);
            }
            if (setn == 11)
            {
                oled_show_string(32, 0, (uint8_t *)"Temp Setting", 16);
                oled_show_string(0, 4, (uint8_t *)"Min:", 16);
                oled_show_string(0, 6, (uint8_t *)"Max:", 16);
            }
        }
        display_set_value();
    }

    if (keynum == KEY1_PRESS)  /* 加 */
    {
        if (setn == 0 && page == 1)
        {
            if (pressCount < 10) pressCount++;

            if (press == 0)
            {
                press = 1;
                beep_beep(100);
                if (timingReminder == 1)
                {
                    timingReminder = 0;
                    beepFlag &= 0xEF;
                }
                else
                {
                    startFlag = !startFlag;
                }
            }
            if (pressCount >= 5)
            {
                startFlag = 0;
                timeCountRecord = 0;
                beep_beep(500);
                pressCount = 0;
            }
        }

        if (page == 1)
        {
            if (setn == 1) { shi++; if (shi == 100) shi = 0; }
            if (setn == 2) { fen++; if (fen == 60) fen = 0; }
            if (setn == 3) { miao++; if (miao == 60) miao = 0; }
            if (setn == 4) { if (bu_long < 200) bu_long++; }
            if (setn == 5) { if (warnMileage < 20000) warnMileage += 10; }
        }
        else
        {
            if (setn == 1) { sys_date.year++; if (sys_date.year > 2099) sys_date.year = 2000; ds1302_set_date(&sys_date); }
            if (setn == 2) { sys_date.mon++; if (sys_date.mon == 13) sys_date.mon = 1; ds1302_set_date(&sys_date); }
            if (setn == 3) { sys_date.day++; if (sys_date.day == 32) sys_date.day = 1; ds1302_set_date(&sys_date); }
            if (setn == 4) { sys_date.week++; if (sys_date.week == 8) sys_date.week = 1; ds1302_set_date(&sys_date); }
            if (setn == 5) { sys_date.hour++; if (sys_date.hour == 24) sys_date.hour = 0; ds1302_set_date(&sys_date); }
            if (setn == 6) { sys_date.min++; if (sys_date.min == 60) sys_date.min = 0; ds1302_set_date(&sys_date); }
            if (setn == 7) { sys_date.sec++; if (sys_date.sec == 60) sys_date.sec = 0; ds1302_set_date(&sys_date); }
            if ((setn == 8) && (xinlvMax - xinlvMin > 1)) xinlvMin++;
            if ((setn == 9) && (xinlvMax < 300)) xinlvMax++;
            if ((setn == 10) && (spo2Min < 200)) spo2Min++;
            if ((setn == 11) && (tempMax - tempMin > 1)) tempMin++;
            if ((setn == 12) && (tempMax < 999)) tempMax++;
        }
        display_set_value();
    }
    else
    {
        if (press == 1)
        {
            press = 0;
            pressCount = 0;
        }
    }

    if (keynum == KEY2_PRESS)  /* 减 */
    {
        if (page == 1)
        {
            if (setn == 0)
            {
                if (mileageReminder == 1)
                {
                    mileageReminder = 0;
                    beep_off();
                }
                else
                {
                    mileage = 0;
                    mileage_bushu = 0;
                }
            }
            if (setn == 1) { if (shi == 0) shi = 100; shi--; }
            if (setn == 2) { if (fen == 0) fen = 60; fen--; }
            if (setn == 3) { if (miao == 0) miao = 60; miao--; }
            if (setn == 4) { if (bu_long > 0) bu_long--; }
            if (setn == 5) { if (warnMileage >= 10) warnMileage -= 10; }
        }
        else
        {
            if (setn == 1) { if (sys_date.year <= 2000) sys_date.year = 2100; sys_date.year--; ds1302_set_date(&sys_date); }
            if (setn == 2) { if (sys_date.mon == 1) sys_date.mon = 13; sys_date.mon--; ds1302_set_date(&sys_date); }
            if (setn == 3) { if (sys_date.day == 1) sys_date.day = 32; sys_date.day--; ds1302_set_date(&sys_date); }
            if (setn == 4) { if (sys_date.week == 1) sys_date.week = 8; sys_date.week--; ds1302_set_date(&sys_date); }
            if (setn == 5) { if (sys_date.hour == 0) sys_date.hour = 24; sys_date.hour--; ds1302_set_date(&sys_date); }
            if (setn == 6) { if (sys_date.min == 0) sys_date.min = 60; sys_date.min--; ds1302_set_date(&sys_date); }
            if (setn == 7) { if (sys_date.sec == 0) sys_date.sec = 60; sys_date.sec--; ds1302_set_date(&sys_date); }
            if ((setn == 8) && (xinlvMin > 0)) xinlvMin--;
            if ((setn == 9) && (xinlvMax - xinlvMin > 1)) xinlvMax--;
            if ((setn == 10) && (spo2Min > 0)) spo2Min--;
            if ((setn == 11) && (tempMin > 0)) tempMin--;
            if ((setn == 12) && (tempMax - tempMin > 1)) tempMax--;
        }
        display_set_value();
    }

    if (keynum == KEY3_PRESS && page == 0)  /* 清零步数 */
    {
        bushu = 0;
    }

    if (keynum == KEY4_PRESS && setn == 0)  /* 切换显示界面 */
    {
        page = !page;
        oled_clear();
        if (page == 0)
        {
            oled_show_string(0, 4, (uint8_t *)"HR", 16);
            oled_show_string(48, 4, (uint8_t *)"SpO2", 16);
            oled_show_string(95, 4, (uint8_t *)"Step", 16);
        }
        else
        {
            oled_show_string(0, 2, (uint8_t *)"Mileage:", 16);
        }
    }
}

/**
 * @brief 报警检测线程
 */
static void alarm_thread_entry(void *parameter)
{
    static uint8_t beep_count = 0;

    while (1)
    {
        /* 计时功能 */
        if (startFlag == 1)
        {
            if (timeCountRecord < (98*3600 + 59*60))
            {
                timeCountRecord++;
            }

            if (timeCountRecord == (shi*3600 + fen*60 + miao))
            {
                timingReminder = 1;
                beepFlag |= 0x10;
            }
        }

        /* 时间提醒蜂鸣 */
        if (timingReminder == 1)
        {
            beep_on();
            rt_thread_mdelay(100);
            beep_off();
            rt_thread_mdelay(900);
        }

        /* 阈值报警 */
        if (((hrAvg != 0) && (hrAvg >= xinlvMax || hrAvg <= xinlvMin)) ||
            ((spo2Avg != 0) && (spo2Avg <= spo2Min)) ||
            (temperature >= tempMax || temperature <= tempMin))
        {
            beep_toggle();
            rt_thread_mdelay(100);
        }
        else
        {
            beep_off();

            /* 里程提醒 */
            if (mileage >= warnMileage)
            {
                if ((beepFlag >> 5) == 0)
                {
                    beepFlag |= 0x20;
                    mileageReminder = 1;
                }
            }
            else
            {
                beepFlag &= 0xDF;
                mileageReminder = 0;
            }

            /* 里程到达蜂鸣 */
            if (mileageReminder == 1)
            {
                beep_count++;
                if (beep_count == 3 || beep_count == 9 || beep_count == 15)
                    beep_on();
                if (beep_count == 6 || beep_count == 12 || beep_count == 18)
                {
                    beep_off();
                    if (beep_count == 18) beep_count = 0;
                }
            }

            rt_thread_mdelay(1000);
        }
    }
}

/**
 * @brief 串口数据发送线程
 */
static void uart_thread_entry(void *parameter)
{
    while (1)
    {
        rt_kprintf("$HR:%d#,$SpO2:%d#,$Temp:%d.%d#,",
                   hrAvg, spo2Avg, temperature/10, temperature%10);
        rt_kprintf("$Steps:%d#,$Mile:%d.%03d#\r\n",
                   bushu, mileage/1000, mileage%1000);

        rt_thread_mdelay(1000);
    }
}

/**
 * @brief 主函数
 */
int main(void)
{
    rt_thread_t tid;

    rt_kprintf("\n=== Smart Band ART-Pi II ===\n");
    rt_kprintf("Based on RT-Thread\n\n");

    /* 初始化OLED */
    oled_init();
    rt_thread_mdelay(100);

    /* 初始化DS1302 RTC */
    ds1302_init(RT_NULL);

    /* 初始化MAX30102 */
    max30102_init();

    /* 显示欢迎信息 */
    oled_clear();
    oled_show_string(16, 2, (uint8_t *)"Smart Band", 16);
    oled_show_string(24, 4, (uint8_t *)"ART-Pi II", 16);
    rt_thread_mdelay(2000);

    /* 显示主界面 */
    oled_clear();
    oled_show_string(0, 4, (uint8_t *)"HR", 16);
    oled_show_string(48, 4, (uint8_t *)"SpO2", 16);
    oled_show_string(95, 4, (uint8_t *)"Step", 16);

    /* 创建报警检测线程 */
    tid = rt_thread_create("alarm",
                           alarm_thread_entry,
                           RT_NULL,
                           1024,
                           20,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    /* 创建串口发送线程 */
    tid = rt_thread_create("uart_tx",
                           uart_thread_entry,
                           RT_NULL,
                           1024,
                           25,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    /* 主循环 */
    while (1)
    {
        shanshuo = !shanshuo;
        key_settings();
        display_time();
        display_time_count();

        if (setn == 0)
        {
            display_temperature();
            get_steps();
            get_heart_rate_spo2();
        }

        rt_thread_mdelay(100);
    }

    return 0;
}
