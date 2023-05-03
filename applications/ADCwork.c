/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-01     Rick       the first version
 */
#include "ADCwork.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "status.h"
#include "key.h"
#include "moto.h"
#include "flashwork.h"
#include "gateway.h"
#include <stm32g0xx_hal.h>
#include "drv_common.h"
#include <sys/errno.h>
#include <math.h>

#define DBG_TAG "adc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define SAMPL_SIZE  5   //方差采样次数

extern enum Device_Status Now_Status;
uint32_t adc_value[4];  //adc值 电位器值 输入电源电压 电容剩余电压 电机电流

rt_adc_device_t adc1_device;    //adc设备结构体
rt_thread_t read_adc = RT_NULL;
rt_sem_t ADC_Convert_Done = RT_NULL;


uint32_t voltage_temp=0;
double adc_voltage;

ADC_HandleTypeDef hadc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};
  ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_SEQ_FIXED;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analog WatchDog 1
  */
  AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_ALL_REG;
  AnalogWDGConfig.ITMode = ENABLE;
  AnalogWDGConfig.HighThreshold = 4095;
  AnalogWDGConfig.LowThreshold = 0;
  if (HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Ch4_7_DMAMUX1_OVR_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Ch4_7_DMAMUX1_OVR_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMAMUX1_OVR_IRQn);

}

/**
  * @brief This function handles ADC1 interrupt.
  */
void ADC1_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_IRQn 0 */

  /* USER CODE END ADC1_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc1);
  /* USER CODE BEGIN ADC1_IRQn 1 */

  /* USER CODE END ADC1_IRQn 1 */
}

void Read_ADC_Callback(void *parameter)
{
//    int i;
//    int sum;        //采样值的和，用于计算样本均值
//    int mean;       //样本均值，即采样值的平均值
//    int poor;       //样本差值，当前值与均值的差
//    int variance;   //样本方差，衡量采样值的离散程度
//
//    while(1)
//    {
//        sum = 0;            //采样值的和，用于计算样本均值
//
//        for (i = 0; i < SAMPL_SIZE; i++) {  // 读取ADC采样值，计算采样值的平方和
//            sum += adc_value[0];
//            rt_thread_mdelay(30);
//        }
//
////        LOG_I("pot = %d\r\t", adc_value[0]);        //电位器
////        LOG_I("in  = %d\r\t", adc_value[1]);        //电源
////        LOG_I("cap = %d\r\t", adc_value[2]);        //电容
////        LOG_I("moot= %d\r\n", adc_value[3]);        //电机电流
//
////        LOG_I("电位 = %d\r\t", adc_value[0]);       //电位器
////        LOG_I("电源 = %d\r\t", adc_value[1]);       //电源
////        LOG_I("电容 = %d\r\t", adc_value[2]);       //电容
////        LOG_I("电机 = %d\r\n", adc_value[3]);       //电机电流
//
//        // 计算样本均值和样本方差
//        mean = (sum / SAMPL_SIZE);
//        poor = (adc_value[0] - mean);
//        variance = pow(poor, 2);
//
//        LOG_I("样本均值: %d\n", mean);
//        LOG_I("样本差值: %d\n", poor);
//        LOG_I("样本方差: %d\n", variance);
//
//        rt_thread_mdelay(300);
//    }

    while(1)
    {
//        LOG_I("电位 = %d\r\t", adc_value[0]);       //电位器
//        LOG_I("电源 = %d\r\t", adc_value[1]);       //电源
        LOG_I("电容 = %d\r\t", adc_value[2]);       //电容
//        LOG_I("电机 = %d\r\n", adc_value[3]);       //电机电流

        rt_thread_mdelay(3000);
    }

}

void ADC_Thread_Init(void)
{
    read_adc = rt_thread_create("read_adc", Read_ADC_Callback, RT_NULL, 1024, 15, 20);
    if(read_adc != RT_NULL);
        rt_thread_startup(read_adc);
}
int ADC_Init(void)
{
//    rt_err_t ret = 0;

    MX_DMA_Init();
    MX_ADC1_Init();
    HAL_ADC_Start_DMA(&hadc1,(uint32_t*) &adc_value, 4);
    LOG_I("ADC Init Success\r\n");
//    ADC_Thread_Init();

    return RT_EOK;
}

void Read_ADC_once(void)
{
    for(int i = 0; i < 5; i++)
    {
        LOG_I("pot = %d\r\t", adc_value[0]);        //电位器
        LOG_I("in  = %d\r\t", adc_value[1]);        //电源
        LOG_I("cap = %d\r\t", adc_value[2]);        //电容
        LOG_I("moot= %d\r\n", adc_value[3]);        //电机电流

        rt_thread_mdelay(1000);
    }
}


/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(Read_ADC_once, Read adc once);

