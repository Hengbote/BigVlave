/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-21     Rick       the first version
 */
#include "stdint.h"

#ifndef APPLICATIONS_DEVICE_H_
#define APPLICATIONS_DEVICE_H_

#define K0_FLAG0            (1 << 0)    //开阀按键事件标志位
#define K0_Long_FLAG1       (1 << 1)    //开阀按键长按事件标志位
#define K1_FLAG2            (1 << 2)    //关阀按键事件标志位
#define K1_Long_FLAG3       (1 << 3)    //关阀按键长按事件标志位
#define K0_K1_Long_FLAG4    (1 << 4)    //双键长按事件标志位

void Key_Event_Init(void);
void Button_Init(void);
void Key_IO_Open(void);     //按键使能标志位开
void Key_IO_Close(void);    //按键使能标志位关


#endif /* APPLICATIONS_DEVICE_H_ */
