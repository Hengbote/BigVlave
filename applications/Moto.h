/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-27     Rick       the first version
 */
#ifndef APPLICATIONS_MOTO_H_
#define APPLICATIONS_MOTO_H_
#include "stdint.h"


enum Motor_Status   //电机状态
{
    TargetClose = 0,    //2730关
    FrontalOpen,        //3800正开
    ReverseOpen,        //1560反开
    TargetStop,         //停止
    NowOpen,            //阀门当前状态(开)
    NowClose            //阀门当前状态(关)
};

void Motor_Clock(void);     //电机顺时针
void Motor_CoClock(void);   //电机逆时针
void Motor_Stop(void);      //电机停止

void Moto_Open(uint8_t ActFlag);
void Moto_Close(uint8_t ActFlag);
void Moto_Init(void);
void Moto_Detect(void);     //电机自检


#endif /* APPLICATIONS_MOTO_H_ */
