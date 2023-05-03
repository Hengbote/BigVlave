/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-27     Rick       the first version
 */
#ifndef APPLICATIONS_STATUS_H_
#define APPLICATIONS_STATUS_H_
#include "stdint.h"


enum Device_Status
{
    Close=0,                    //设备关闭
    Open,                       //设备开启
    SlaverLowPower,             //从机低功耗状态
    SlaverUltraLowPower,        //从机超低功耗状态
    SlaverWaterAlarmActive,     //从机水浸告警状态
    MasterLostPeak,             //主机失峰状态
    MasterWaterAlarmActive,     //主机水浸告警状态
    MasterWaterAlarmDeActive,   //主机水浸告警解除状态
    NTCWarning,                 //温度传感器故障状态
    MotoFail,                   //电机异常状态
    Learn,                      //学习模式状态(配对网关)
    Offline                     //设备离线状态
};
typedef struct
{
    uint8_t warning_id;
    uint8_t last_id;
    uint8_t priority;
    void (*callback)(void*);
}WariningEvent;

void WarningInit(void);
void BackToNormal(void);
void Warning_Disable(void);
void Warning_Enable_Num(uint8_t id);
void MasterAlarmWaterDisable(void);
void MasterStatusChangeToDeAvtive(void);
void OfflineDisableWarning(void);
void RadioInitFail(void);
void Delay_Timer_Init(void);
void Delay_Enable(void);
void Delay_Disable(void);
void KidLock_Enable(void);
void KidLock_Disable(void);
uint8_t Detect_Learn(void);
void Delay_Timer_Stop(void);
void Delay_Timer_OpenDoor(uint32_t device_id);
void Delay_Timer_CloseDoor(uint32_t device_id);
void Remote_Open(void);
void Remote_Close(void);
uint8_t GetNowStatus(void);
#endif /* APPLICATIONS_STATUS_H_ */
