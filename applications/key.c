/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-25     Rick       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "pin_config.h"
#include "key.h"
#include "led.h"
#include "moto.h"
#include "Radio_Decoder.h"
#include "Radio_encoder.h"
#include "work.h"
#include "status.h"
#include "flashwork.h"
#include "rthw.h"
#include "status.h"
#include "device.h"
#include "gateway.h"
#include "factory.h"

#define DBG_TAG "key"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

rt_thread_t key_response_t = RT_NULL;
rt_timer_t Learn_Timer = RT_NULL;

extern rt_event_t Key_event;                //按键事件结构体
rt_uint32_t key_recv;                       //实际接收到的事件标志值

uint8_t Valve_Status = 0;                   //阀门当前状态 NowClose关 NowOpen开
uint8_t Valve_target_Status = TargetClose;  //阀门目标状态 0关 1正开 2反开 3停止

extern uint8_t Learn_Flag;
extern uint8_t Last_Close_Flag;

extern enum Device_Status Now_Status;
extern uint8_t Factory_Flag;

void Key_Reponse_Callback(void *parameter)
{
    Key_Event_Init();
    LOG_D("Key_Reponse Init Success\r\n");
    while(1)
    {
        /*
         * K0_FLAG0            (1 << 0)    //开阀按键事件标志位
         * K0_Long_FLAG1       (1 << 1)    //开阀按键长按事件标志位
         * K1_FLAG2            (1 << 2)    //关阀按键事件标志位
         * K1_Long_FLAG3       (1 << 3)    //关阀按键长按事件标志位
         * K0_K1_Long_FLAG4    (1 << 4)    //双键长按事件标志位
                  *  逻辑与参数 | 清除参数
                  *  永远阻塞直到获得资源
         */
        rt_event_recv(  Key_event,
                        (K0_FLAG0 | K0_Long_FLAG1 | K1_FLAG2 | K1_Long_FLAG3 | K0_K1_Long_FLAG4),
                        RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                        RT_WAITING_FOREVER,
                        &key_recv);

        if(key_recv & K0_FLAG0)//ON
        {
            Moto_Open(NormalOpen);

//            switch(Now_Status)
//            {
//            case Close:
//                if(Last_Close_Flag==0)
//                {
//                    just_ring();
//                    Moto_Open(NormalOpen);
//                }
//                else
//                {
//                    beep_start(0,7);//蜂鸣器三下
//                }
//                LOG_D("Valve Open With ON\r\n");
//                break;
//            case Open:
//                just_ring();
//                LOG_D("Valve Already Open With ON\r\n");
//                break;
//            case SlaverLowPower:
//                break;
//            case SlaverUltraLowPower:
//                beep_three_times();
//                break;
//            case SlaverWaterAlarmActive:
//                break;
//            case MasterLostPeak:
//                key_down();
//                Now_Status = Open;
//                Moto_Open(NormalOpen);
//                LOG_D("MasterLostPeak With ON\r\n");
//                break;
//            case MasterWaterAlarmActive:
//                beep_three_times();
//                break;
//            case MasterWaterAlarmDeActive:
//                beep_three_times();
//                LOG_D("MasterWaterAlarmActive With ON\r\n");
//                break;
//            case MotoFail:
//                just_ring();
//                break;
//            case Learn:
//                break;
//            case Offline:
//                break;
//            case NTCWarning:
//                break;
//            }
        }
        else if(key_recv & K1_FLAG2)//OFF
        {
            Moto_Close(NormalOff);

//            if(Factory_Flag)
//            {
//                Stop_Factory_Cycle();
//                Warning_Disable();
////                Moto_Detect();
//            }
//            else
//            {
//                switch(Now_Status)
//                {
//                case Close:
//                    if(Last_Close_Flag==0)
//                    {
//                        key_down();
//                    }
//                    else
//                    {
//                        beep_start(0,7);//蜂鸣器三下
//                    }
//                    LOG_D("Valve Already Close With OFF\r\n");
//                    break;
//                case Open:
//                    key_down();
//                    Last_Close_Flag = 0;
//                    Moto_Close(NormalOff);
//                    LOG_D("Valve Close With OFF\r\n");
//                    break;
//                case SlaverLowPower:
//                    break;
//                case SlaverUltraLowPower:
//                    just_ring();
//                    break;
//                case SlaverWaterAlarmActive:
//                    beep_stop();
//                    break;
//                case MasterLostPeak:
//                    key_down();
//                    Moto_Close(NormalOff);
//                    beep_stop();
//                    Now_Status = Close;
//                    LOG_D("MasterLostPeak With OFF\r\n");
//                    break;
//                case MasterWaterAlarmActive:
//                    beep_stop();
//                    break;
//                case MasterWaterAlarmDeActive:
//                    key_down();
//                    Now_Status = Close;
//                    Warning_Disable();
//                    WarUpload_GW(1,0,1,0);//主控消除水警
//                    LOG_D("MasterWaterAlarmActive With OFF\r\n");
//                    break;
//                case Learn:
//                    break;
//                case MotoFail:
//                    key_down();
//                    LOG_D("MotoFail With OFF\r\n");
//                    break;
//                case Offline:
//                    break;
//                case NTCWarning:
//                    beep_stop();
//                    key_down();
//                    break;
//                }
//            }
        }
        else if(key_recv & K0_K1_Long_FLAG4)
        {
//            DeleteAllDevice();
//            beep_start(0,8);//蜂鸣器5次
//            rt_thread_mdelay(3000);
//            rt_hw_cpu_reset();
        }
        else if(key_recv & K0_Long_FLAG1)//ON
        {
        }
        else if(key_recv & K1_Long_FLAG3)//OFF
        {
//            if(Now_Status==Close||Now_Status==Open)
//            {
//                Now_Status = Learn;
//                Start_Learn_Key();
//            }
//            else if(Now_Status==Learn)
//            {
//                rt_timer_stop(Learn_Timer);
//                Stop_Learn();
//            }
//            else
//            {
//                LOG_D("Now in Warining Mode\r\n");
//            }
        }
        rt_thread_mdelay(10);
    }
}
void Learn_Timer_Callback(void *parameter)
{
    LOG_D("Learn timer is Timeout\r\n");
    Stop_Learn();
}
void Key_Reponse(void)
{
    key_response_t = rt_thread_create("key_response_t", Key_Reponse_Callback, RT_NULL, 2048, 10, 10);
    if(key_response_t!=RT_NULL)rt_thread_startup(key_response_t);
    Learn_Timer = rt_timer_create("Learn_Timer", Learn_Timer_Callback, RT_NULL, 30*1000, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER );
    LOG_I("Key_Reponse Init Success\r\n");
}
