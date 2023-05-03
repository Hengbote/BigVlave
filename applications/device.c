/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-21     Rick       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "button.h"
#include "device.h"
#include "pin_config.h"
#include "led.h"
#include "moto.h"

#define DBG_TAG "device"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

rt_event_t Key_event;       //按键事件结构体

rt_thread_t button_task = RT_NULL;

extern uint8_t Hand_Status;         //手动轴目前状态 (同时控制按键任务是否使能) 0拔起拨动 1放下

uint16_t K0_Long_Sem_Counter = 0;   //开阀按键长按计数器
uint16_t K1_Long_Sem_Counter = 0;   //关阀按键长按计数器
uint8_t K0_OnceFlag = 0;            //开阀按键单次标志位
uint8_t K1_OnceFlag = 0;            //关阀按键单次标志位
uint8_t K0_K1_OnceFlag = 0;         //双键单次标志位

uint8_t Key_Pause_Flag;             //按键使能标志位 1关0开

extern uint8_t Factory_Flag;        //工厂标志位

void Key_Event_Init(void)   //初始化按键事件
{
    Key_event = rt_event_create ("Key_event", RT_IPC_FLAG_FIFO);
    if (Key_event == RT_NULL)
    {
        rt_kprintf("init event failed.\n");
    }
}

void Key_Pin_Init(void)     //初始化按键引脚
{
    rt_pin_mode(K0, PIN_MODE_INPUT);
    rt_pin_mode(K1, PIN_MODE_INPUT);
}

void Key_IO_Open(void)      //按键停止标志位开
{
    Key_Pause_Flag = 0;
}
void Key_IO_Close(void)     //按键停止标志位关
{
    Key_Pause_Flag = 1;
}

void K0_Event_Send(void *parameter)
{
    /* 发送开阀按键事件标志 */
    rt_event_send(Key_event, K0_FLAG0);
    LOG_I("K0 is Down\r\n");
}
void K1_Event_Send(void *parameter)
{
    /* 发送关阀按键事件标志 */
    rt_event_send(Key_event, K1_FLAG2);
    LOG_I("K1 is Down\r\n");
}

void K0_Long_Event_Send(void *parameter)
{
    if (K0_OnceFlag == 0)
    {
        if (K0_Long_Sem_Counter > 6)
        {
            if (K1_Long_Sem_Counter == 0)
            {
                K0_OnceFlag = 1;
                /* 发送开阀按键长按事件标志 */
                rt_event_send(Key_event, K0_Long_FLAG1);
                LOG_I("K0 is Long\r\n");
            }
        }
        else
        {
            LOG_I("K0 Long Counter is %d", K0_Long_Sem_Counter++);
        }
    }
}
void K1_Long_Event_Send(void *parameter)
{
    if (K1_OnceFlag == 0)
    {
        if (K1_Long_Sem_Counter > 6)
        {
            K1_OnceFlag = 1;
            if (K0_Long_Sem_Counter > 5)
            {
                /* 发送双键长按事件标志 */
                rt_event_send(Key_event, K0_K1_Long_FLAG4);
                LOG_I("K0_K1 is Long\r\n");
            }
            else
            {
                /* 发送关阀按键长按事件标志 */
                rt_event_send(Key_event, K1_Long_FLAG3);
                LOG_I("K1 is Long\r\n");
            }
        }
        else
        {
            LOG_I("K1 Long Counter is %d", K1_Long_Sem_Counter++);
        }
    }
}
void k0_K1_Long_Event_Send(void)    //释放双键长按信号量
{
    if (K0_K1_OnceFlag == 0)
    {
        K0_K1_OnceFlag = 1;
        /* 发送双键长按事件标志 */
        rt_event_send(Key_event, K0_K1_Long_FLAG4);
        LOG_I("K0_K1 is Down\r\n");
    }
}
void K0_LongFree_Release(void *parameter)
{
    K0_OnceFlag = 0;
    K0_K1_OnceFlag = 0;
    K0_Long_Sem_Counter = 0;
    LOG_I("K0 is LongFree\r\n");
}

void K1_LongFree_Release(void *parameter)
{
    K1_OnceFlag = 0;
    K1_Long_Sem_Counter = 0;
    LOG_I("K1 is LongFree\r\n");
}

uint8_t Read_K0_Level(void)
{
    if(Key_Pause_Flag)
    {
        return 1;
    }
    else
    {
        return rt_pin_read(K0);
    }
}

uint8_t Read_K1_Level(void)
{
    if(Key_Pause_Flag)
    {
        return 1;
    }
    else
    {
        return rt_pin_read(K1);
    }
}

void button_task_entry(void *parameter)
{
    Key_Pin_Init();
    Button_t Key0;
    Button_t Key1;
    Button_Create("Key0", &Key0, Read_K0_Level, 0);
    Button_Create("Key1", &Key1, Read_K1_Level, 0);
    Button_Attach(&Key0, BUTTON_DOWM, K0_Event_Send);
    Button_Attach(&Key1, BUTTON_DOWM, K1_Event_Send);
    Button_Attach(&Key0, BUTTON_LONG, K0_Long_Event_Send);
    Button_Attach(&Key1, BUTTON_LONG, K1_Long_Event_Send);
    Button_Attach(&Key0, BUTTON_LONG_FREE, K0_LongFree_Release);
    Button_Attach(&Key1, BUTTON_LONG_FREE, K1_LongFree_Release);
    LOG_I("Button Init Success\r\n");
    while (1)
    {
        Button_Process();
        rt_thread_mdelay(10);
    }
}
void Button_Init(void)
{
    button_task = rt_thread_create("button_task", button_task_entry, RT_NULL, 1024, 5, 10);
    rt_thread_startup(button_task);
}

void Rd_Key_Flag(void)
{
    LOG_I("Key_Pause_Flag = %d\r\n", Key_Pause_Flag);
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(Rd_Key_Flag, Key_Pause_Flag);
