/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-01     Rick       the first version
 */
#ifndef APPLICATIONS_ADCWORK_H_
#define APPLICATIONS_ADCWORK_H_

void ADC1_IRQHandler(void);

void Read_ADC_Callback(void *parameter);
void ADC_Thread_Init(void);
int ADC_Init(void);
void Read_ADC_once(void);


#endif /* APPLICATIONS_ADCWORK_H_ */
