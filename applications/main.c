/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-06-05     RT-Thread    first version
 */
//删除了syswatch
//自己写的rtc
//修改adc cubemx
//按键从信号量改成事件
//手动轴信号量改标志位
//完成motor

//修改rtc

//spi flash
////

#include <rtthread.h>
#include <rtdevice.h>
#include "ax5043.h"
#include "device.h"
#include "led.h"
#include "work.h"
#include "easyflash.h"
#include "flashwork.h"
#include "key.h"
#include "moto.h"
#include "RTCWork.h"
#include "status.h"
#include "adcwork.h"
#include "radio_decoder.h"
#include "gateway.h"
#include "factory.h"

#include "pin_config.h"
#include "board.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define MCU_VER "1.2.1"

int main(void)
{

//    LOG_I("System Version is %s\r\n",MCU_VER);
    led_Init();
    Key_Reponse();
    flash_Init();     //
    WarningInit();
    RTC_Init();
//    RF_Init();        //
    Button_Init();
    ADC_Init();
    Moto_Init();
//    WaterScan_Init();
//    DetectFactory();  //工厂模式不能测试
//    Gateway_Init();   //网关
    while (1)
    {
//        LOG_D("Hello RT-Thread!");
        rt_thread_mdelay(1000);
    }
    return RT_EOK;
}
