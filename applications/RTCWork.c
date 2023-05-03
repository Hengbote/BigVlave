#include <rtthread.h>
#include <rtdevice.h>
#include "pin_config.h"
#include "RTCWork.h"
#include "Flashwork.h"
#include "moto.h"
#include "drv_common.h"

#define DBG_TAG "RTC"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

uint8_t RTC_Counter=0;  //RTC计数器
uint32_t RTC_Hours = 0; //RTC小时

rt_sem_t RTC_IRQ_Sem;   //RTC中断信号量
rt_thread_t RTC_Scan = RT_NULL; //RTC线程句柄
RTC_HandleTypeDef hrtc; //rtc初始化结构体
void RTC_Timer_Entry(void *parameter)
{
    while(1)
    {
        static rt_err_t result;
        result = rt_sem_take(RTC_IRQ_Sem, RT_WAITING_FOREVER);  //获取RTC中断信号量 永远阻塞直到获得资源
        if (result == RT_EOK)
        {
//            if(RTC_Hours%5==0)
//                Moto_Detect();//电机检测
            if(RTC_Hours%120==0)
                Moto_Detect();//电机检测

            if(RTC_Counter<24)
            {
//                Update_All_Time();//24小时更新全部时间
                RTC_Counter++;
            }
            else
            {
//                Update_All_Time();//24小时更新全部时间
//                Detect_All_Time();//25个小时检测计数器
                RTC_Counter=0;
            }
//            LOG_I("Device RTC Hours,Hour is %d\r\n",RTC_Hours);
//            LOG_I("Device RTC Detect,Hour is %d\r\n",RTC_Counter);
        }
    }
}

/* RTC init function */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;                                  //指定要使用RTC设备
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;             //24小时格式
  hrtc.Init.AsynchPrediv = 127;                         //指定RTC异步预分频器的分频系数为127  (它用于将RTC时钟源的频率降低到一个可以处理的频率)
  hrtc.Init.SynchPrediv = 255;                          //指定RTC同步预分频器的分频系数为255 (它用于将异步预分频器的时钟降低到一个更低的频率)
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;                //指定RTC输出模式被禁用
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;        //指定RTC输出_不需要_重映射模式
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;  //指定RTC输出模式的极性为高电平
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;     //指定RTC输出模式的输出类型为开漏输出
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;      //指定RTC输出模式的上拉电阻为不需要
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
      LOG_W("RTC Init Fail\r\n");
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;                                  //小时数初始化为0
  sTime.Minutes = 0;                                //分钟数初始化为0
  sTime.Seconds = 0;                                //秒数初始化为0
  sTime.SubSeconds = 0;                             //子秒数初始化为0
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;   //设定初始化为无夏令时
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;  //存储操作初始化为复位
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
      LOG_W("RTC Set Time Fail\r\n");
  }

  sDate.WeekDay = RTC_WEEKDAY_MONDAY;   //将星期几设置为周一
  sDate.Month = RTC_MONTH_JANUARY;      //将月份设置为一月
  sDate.Date =  1;                      //将日期设置为1日
  sDate.Year =  0;                      //将年份设置为00年
  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
      LOG_W("RTC Set Date Fail\r\n");
  }

  /** Enable the Alarm A
  */

//  sAlarm.AlarmTime.Hours = 0;                                   //设置闹钟时间的小时数为0
//  sAlarm.AlarmTime.Minutes = 0;                                 //设置闹钟时间的分钟数为0
//  sAlarm.AlarmTime.Seconds = 4;                                //设置闹钟时间的秒数
//  sAlarm.AlarmTime.SubSeconds = 0;                              //设置闹钟时间的亚秒数为0

  //闹钟设置成了每个星期一的1:00都会触发
  sAlarm.AlarmTime.Hours = 1;                                   //设置闹钟时间的小时数为1
  sAlarm.AlarmTime.Minutes = 0;                                 //设置闹钟时间的分钟数为0
  sAlarm.AlarmTime.Seconds = 0;                                 //设置闹钟时间的秒数为0
  sAlarm.AlarmTime.SubSeconds = 0;                              //设置闹钟时间的亚秒数为0

  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;    //设置闹钟时间的夏令时模式为无夏令时
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;   //设置闹钟时间的存储操作为重置存储
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;                 //设置闹钟掩码为星期
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;       //设置闹钟亚秒掩码为全匹配，即亚秒也要匹配
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY; //设置闹钟匹配的日期或星期为星期几
  sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;                 //设置闹钟匹配的星期为星期一
  sAlarm.Alarm = RTC_ALARM_A;                                   //设置闹钟为A类型闹钟
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
      LOG_W("RTC Set Alarm Fail\r\n");
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *RtcHandle)  //HAL库提供的RTC闹钟事件回调函数
{
    RTC_TimeTypeDef sTime = {0};
    sTime.Hours = 0;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    //将sTime结构体变量的小时、分钟和秒数分别设置为0
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;     //为无夏令
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;    //存储操作为重置存储

    HAL_RTC_SetTime(RtcHandle, &sTime, RTC_FORMAT_BIN); //将RTC的时间设置为sTime所表示的时间

    rt_sem_release(RTC_IRQ_Sem);
    RTC_Hours++;
}

void RTC_Init(void)
{
//    __HAL_RCC_RTC_ENABLE();                     //使能RTC时钟
//    __HAL_RCC_RTCAPB_CLK_ENABLE();              //RTC时钟总线

    RTC_IRQ_Sem = rt_sem_create("RTC_IRQ", 0, RT_IPC_FLAG_FIFO);    //创建RTC中断信号量 先进先出模式
    RTC_Scan = rt_thread_create("RTC_Scan", RTC_Timer_Entry, RT_NULL, 2048, 5, 5);  //创建线程
    if(RTC_Scan!=RT_NULL)
    {
        rt_thread_startup(RTC_Scan);    //启动线程
    }
    else
    {
        LOG_W("RTC Init Fail\r\n");
    }

    MX_RTC_Init();

    HAL_NVIC_SetPriority(RTC_TAMP_IRQn, 0, 0);  //设置中断优先级
    HAL_NVIC_EnableIRQ(RTC_TAMP_IRQn);          //启用 RTC Tamper中断检测

    LOG_I("RTC Init Success\r\n");
}

void RTC_TAMP_IRQHandler(void)
{
    HAL_RTC_AlarmIRQHandler(&hrtc);
}

void Read_RTC_once(void)
{
    while(1)
    {
        // 读取时间和日期
        RTC_TimeTypeDef sTime;
        RTC_DateTypeDef sDate;
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    //    LOG_I("%04d/%02d/%02d %02d:%02d:%02d 星期%d\r\n",
        LOG_I("%04d/%02d/%02d %02d:%02d:%02d\r\n",
                sDate.Year + 2000,  //年
                sDate.Month,        //月
                sDate.Date,         //日
    //            sDate.WeekDay,      //星期
                sTime.Hours,        //时
                sTime.Minutes,      //分
                sTime.Seconds       //秒
        );

        rt_thread_mdelay(1000);
    }
}


/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(Read_RTC_once, Read rtc once);

