/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-25     Rick       the first version
 */
#ifndef APPLICATIONS_PIN_CONFIG_H_
#define APPLICATIONS_PIN_CONFIG_H_
//RADIO
#define Radio_IRQ               17      //RF        //
//#define ANT_EXT                 26      //外部天线使能(b10)
//#define ANT_INT                 18      //内部天线使能(b2)
//#define ANT_SW                  113     //内外天线开关(h1)
//BASIC
#define K0                      21      //on        //
#define K1                      22      //off       //
#define HAND                    4       //手动轴开关 //未完成

#define BEEP_PIN                8       //蜂鸣器           //
#define LED0_PIN                50      //on开           //
//#define LED1_PIN                21
//#define LED2_PIN                22
#define LED3_PIN                48      //off关          //
#define LED4_PIN                49      //wifi正常           //
//#define LED5_PIN                49      //wifi报警      //
#define Peak_ON                 29      //漏水(三极管)(b13)      //
//#define Peak_Loss               19      //漏水(没有
//#define Turn1                   9       //
//#define Turn2                   11      //电机1
//#define Senor1                  8       //
//#define Senor2                  10      //电机2
#define MOT1                    30      //电机1   //未完成
#define MOT2                    31      //电机2   //未完成
#define TEST                    45      //测试(c13)               //



#endif /* APPLICATIONS_PIN_CONFIG_H_ */
