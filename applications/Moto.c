/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-27     Rick       the first version
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "pin_config.h"
#include "led.h"
#include "key.h"
#include "moto.h"
#include "flashwork.h"
#include "status.h"
#include "gateway.h"
#include "radio_encoder.h"
#include "device.h"
#include "work.h"
#include "ADCwork.h"

#define DBG_TAG "moto"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define FRON_OPEN_POTE 3800     //正开电位
#define REVE_OPEN_POTE 1560     //反开电位
#define TAR_CLOSE_POTE 2730     //关阀电位

#define SAMPL_SIZE  5           //方差采样次数

uint8_t Hand_Status=1;          //手动轴目前状态 (同时控制按键任务是否使能) 0拔起拨动 1放下
uint8_t Hand_Tarhet_Flag=0;     //手动轴目标标志位

extern uint32_t adc_value[4];   //ADC值 电位器值 输入电源电压 电容剩余电压 电机电流
extern uint8_t Valve_Status;    //阀门当前状态
extern uint8_t Valve_target_Status;    //阀门目标状态 0关 1正开 2反开 3开
extern enum Device_Status Now_Status;
extern Device_Info Global_Device;

rt_thread_t moto_work = RT_NULL;            //电机工作任务结构体
rt_timer_t Moto_Self_Test_Timer = RT_NULL;  //电机自检定时器

void Motor_Open_Judgment(void)
{
    if((2600 < adc_value[0]) && (adc_value[0] < 4095))    //判断目前是正开快还是反开快
    {
        Valve_target_Status = FrontalOpen;  //3800正开
        Hand_Tarhet_Flag = FrontalOpen;

        if(2600 < adc_value[0] && adc_value[0] < FRON_OPEN_POTE){
            rt_pin_write(MOT1, 1);  //逆时针 电位器变大
            rt_pin_write(MOT2, 0);
        }
        else {
            rt_pin_write(MOT1, 0);  //顺时针 电位器变小
            rt_pin_write(MOT2, 1);
        }
    }
    else
    {
        Valve_target_Status = ReverseOpen;  //1560反开
        Hand_Tarhet_Flag = ReverseOpen;

        if(REVE_OPEN_POTE > adc_value[0] || adc_value[0] > 4000){
            rt_pin_write(MOT1, 1);  //逆时针 电位器变大
            rt_pin_write(MOT2, 0);
        }
        else {
            rt_pin_write(MOT1, 0);  //顺时针 电位器变小
            rt_pin_write(MOT2, 1);
        }
    }
}

void Motor_Close_Judgment(void)
{
    Valve_target_Status = TargetClose;  //2730关
    Hand_Tarhet_Flag = TargetClose;

    if(TAR_CLOSE_POTE < adc_value[0]){
        rt_pin_write(MOT1, 0);  //顺时针 电位器变小
        rt_pin_write(MOT2, 1);
    }
    else {
        rt_pin_write(MOT1, 1);  //逆时针 电位器变大
        rt_pin_write(MOT2, 0);
    }
}

void Moto_Open(uint8_t mode)
{
    if(adc_value[2] > 1200)     //判断电容是否有电
    {
        LOG_I("Moto is Open\r\n");
        Now_Status = Open;
        led_Long_Start(1);//绿灯
        Motor_Open_Judgment();  //阀门开------------
        just_ring();
    }
}
void Moto_Close(uint8_t ActFlag)
{
    if(adc_value[2] > 1200)     //判断电容是否有电
    {
        LOG_I("Moto is Close\r\n");
        Now_Status = Close;
        led_Stop(1);//绿灯
        Motor_Close_Judgment();  //阀门关----------
        just_ring();
    }
}

void moto_work_Callback(void *parameter)
{
    int i;
    int sum;        //采样值的和，用于计算样本均值
    int mean;       //样本均值，即采样值的平均值
    int poor;       //样本差值，当前值与均值的差
    int variance;   //样本方差，衡量采样值的离散程度
    uint8_t Hand_St=1;

    rt_pin_mode(HAND, PIN_MODE_INPUT);
    while(1)
    {
        sum = 0;            //采样值的和，用于计算样本均值

        Hand_Status = rt_pin_read(HAND);
        if(Hand_Status == 0)
        {
            Valve_target_Status = Hand_Tarhet_Flag; //记录电机目标状态
            Hand_St = 1;
            rt_pin_write(MOT1, 0);  //停止电机
            rt_pin_write(MOT2, 0);
            Key_IO_Close();    //按键使能标志位 关
        }
        else if(Hand_Status == 1)
        {
            if(adc_value[2] > 1200)     //判断电容是否有电
            {
                if(Hand_St)
                {
                    Hand_St = 0;

                    if(Valve_target_Status == TargetClose)
                        Motor_Close_Judgment();
                    else if(Valve_target_Status == FrontalOpen)
                        Motor_Open_Judgment();
                    else if(Valve_target_Status == ReverseOpen)
                        Motor_Open_Judgment();
                    else if(Valve_target_Status == TargetStop)
                    {
                        rt_pin_write(MOT1, 0);
                        rt_pin_write(MOT2, 0);
                    }
                    Key_IO_Open();     //按键使能标志位 开
                }
            }
        }

        for (i = 0; i < SAMPL_SIZE; i++) {  // 读取ADC采样值，计算采样值的平方和
            sum += adc_value[0];
            rt_thread_mdelay(30);
        }

//        LOG_I("电位 = %d\r\t", adc_value[0]);       //电位器
//        LOG_I("电源 = %d\r\t", adc_value[1]);       //电源
//        LOG_I("电容 = %d\r\t", adc_value[2]);       //电容
//        LOG_I("电机 = %d\r\n", adc_value[3]);       //电机电流
//        LOG_I("电机状态= %d\r\n", Valve_target_Status);

        // 计算样本均值和样本方差
        mean = (sum / SAMPL_SIZE);
        poor = (adc_value[0] - mean);
        variance = poor * poor;

//        LOG_I("样本方差: %d\n", variance);

        if(Valve_target_Status == FrontalOpen)      //3800正开
        {
            if(variance < 226)    //是否在死区
            {
                if(((FRON_OPEN_POTE - 40) <= adc_value[0]) && (adc_value[0] <= (FRON_OPEN_POTE + 40)))
                {
                    rt_pin_write(MOT1, 0);  //停止
                    rt_pin_write(MOT2, 0);
                    Valve_target_Status = TargetStop;
                    Valve_Status = NowOpen;
                }
            }
        }
        else if(Valve_target_Status == ReverseOpen) //1560反开
        {
            if(variance < 226)    //是否在死区
            {
                if(((REVE_OPEN_POTE - 40) <= adc_value[0]) && (adc_value[0] <= (REVE_OPEN_POTE + 40)))
                {
                    rt_pin_write(MOT1, 0);  //停止
                    rt_pin_write(MOT2, 0);
                    Valve_target_Status = TargetStop;
                    Valve_Status = NowOpen;
                }
            }
        }
        else if(Valve_target_Status == TargetClose) //2730关
        {
            if(variance < 226)    //是否在死区
            {
                if(((TAR_CLOSE_POTE - 20) <= adc_value[0]) && (adc_value[0] <= (TAR_CLOSE_POTE + 20)))
                {
                    rt_pin_write(MOT1, 0);  //停止
                    rt_pin_write(MOT2, 0);
                    Valve_target_Status = TargetStop;
                    Valve_Status = NowClose;
                }
            }
        }

        rt_thread_mdelay(10);
    }
}

void Moto_Self_Test_Timer_Callback(void *parameter)
{
    rt_pin_write(MOT1, 0);  //停止
    rt_pin_write(MOT2, 0);
    LOG_I("电位 = %d\r\t", adc_value[0]);       //电位器
    Motor_Open_Judgment();  //阀门开------------
}

void Moto_Init(void)
{
    rt_pin_mode(MOT1,PIN_MODE_OUTPUT);
    rt_pin_mode(MOT2,PIN_MODE_OUTPUT);

    Moto_Self_Test_Timer = rt_timer_create("Moto_Self_Test_Timer", Moto_Self_Test_Timer_Callback, RT_NULL, 3000, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);  //一次性计时器 软件定时器，定时器的回调函数将在定时器线程中调用

    moto_work = rt_thread_create("moto_work", moto_work_Callback, RT_NULL, 1024, 15, 20);
    if(moto_work != RT_NULL);
        rt_thread_startup(moto_work);

    LOG_I("Moto is Init Success\r\n");
}

void Moto_Detect(void)
{
    if(Valve_Status == NowOpen)
    {
//        Turn1_Flag = 0;
//        Moto1_Fail_FLag = 0;

        Key_IO_Close();    //按键使能标志位 关
//        WaterScan_IO_DeInit();
        rt_timer_start(Moto_Self_Test_Timer);
        rt_pin_write(MOT1, 0);  //顺时针 电位器变小
        rt_pin_write(MOT2, 1);
    }
}


void Motor_Clock(void)      //电机顺时针
{
    rt_pin_write(MOT1, 0);  //顺时针 电位器变小
    rt_pin_write(MOT2, 1);
}

void Motor_CoClock(void)    //电机逆时针
{
    rt_pin_write(MOT1, 1);  //逆时针 电位器变大
    rt_pin_write(MOT2, 0);
}

void Motor_Stop(void)       //电机停止
{
    rt_pin_write(MOT1, 0);  //逆时针
    rt_pin_write(MOT2, 0);
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(Motor_Clock, Motor clockwise);
MSH_CMD_EXPORT(Motor_CoClock, Motor counterclockwise);
MSH_CMD_EXPORT(Motor_Stop, Motor stop);
