终端通过  按键事件发生组播数据，协调器只接收
  if ( keys & HAL_KEY_SW_6 )
  {
    /* This key sends the Flash Command is sent to Group 1.
     * This device will not receive the Flash Command from this
     * device (even if it belongs to group 1).
     */
     #if defined(ZDO_COORDINATOR)  //协调器只接收数据
     
     #else                         //路由器和终端才会发送数据
        SampleApp_SendFlashMessage(0); //以组播方式发送数据
     #endif
  }


组播发送函数将小灯状态翻转LedState，然后发送给协调器
void SampleApp_SendFlashMessage( uint16 flashTime ) //此实验没有用到，后面再分析
{
  LedState = ~LedState;

  if ( AF_DataRequest( &SampleApp_Flash_DstAddr, 
                       &SampleApp_epDesc,
                       SAMPLEAPP_FLASH_CLUSTERID,
                       1,
                       &LedState,
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
     if(LedState == 0)
      {
        HalLedSet(HAL_LED_2,HAL_LED_MODE_OFF);
      }
      else
      {
        HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
      }
  }
  else
  {
    // Error occurred in request to send.
  }
  
  
}


协调器在接收到数据时，判断接收的数据并作出相应的状态
    case SAMPLEAPP_FLASH_CLUSTERID: //收到组播数据
      data = (uint8)pkt->cmd.Data[0];
      if(data == 0)
      {
        HalLedSet(HAL_LED_2,HAL_LED_MODE_OFF);
      }
      else
      {
        HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
      }
      break;
      
################################################################################      


在左边workspace目录下比较重要的两个文件夹分别是Zmain和App。我们开发主要在App文件
夹进行，这也是用户自己添加自己代码的地方。主要修改SampleApp.c和SampleApp.h即可，
如果增加传感器则增加相应的模块驱动到App里面，在SampleApp.c中调用就行。

SampleApp.c文件SampleApp_Init()函数中;
//------------------------------ 配置串口 --------------------------------------
  MT_UartInit();   //初始化串口
  MT_UartRegisterTaskID(task_id); //注册串口任务
  HalUARTWrite(0,"Hello world\n",sizeof("Hello world\n")); //向串口发送数据
//------------------------------------------------------------------------------

MT_UART.c文件中MT_UartInit()函数中
 //设置波特率
 uartConfig.baudRate             = MT_UART_DEFAULT_BAUDRATE;
 //设置是否有流控制
 uartConfig.flowControl          = MT_UART_DEFAULT_OVERFLOW;
 
MT_UART.h文件中修改MT_UART_DEFAULT_BAUDRATE、MT_UART_DEFAULT_OVERFLOW来修改波特率
和流控制
#if !defined( MT_UART_DEFAULT_OVERFLOW )
  #define MT_UART_DEFAULT_OVERFLOW       FALSE
#endif

#if !defined MT_UART_DEFAULT_BAUDRATE
#define MT_UART_DEFAULT_BAUDRATE         HAL_UART_BR_115200
#endif
  
  
    //读取串口数据
    HalUARTRead(0,ch,4);
    
    

/*****************************************************************************
* 实验中协调器只接收数据所以取消发送事件，ZDO_STATE_CHANGE在这里设置
******************************************************************************/

case ZDO_STATE_CHANGE:
//只要网络状态发生改变，就通过ZDO_STATE_CHANGE事件通知所有的任务。
//同时完成对协调器，路由器，终端的设置
SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
//if ( (SampleApp_NwkState == DEV_ZB_COORD)//实验中协调器只接收数据所以取消发送事件
if ( (SampleApp_NwkState == DEV_ROUTER) || (SampleApp_NwkState == DEV_END_DEVICE) )
{
  // Start sending the periodic message in a regular interval.
  //这个定时器只是为发送周期信息开启的，设备启动初始化后从这里开始
  //触发第一个周期信息的发送，然后周而复始下去
  osal_start_timerEx( SampleApp_TaskID,
                    SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
                    SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT );
}
else
{
  // Device is no longer in the network
}
break;



/*****************************************************************************
*  函数名称  ： osal_start_timerEx
*  函数介绍  ： 这个函数提供定时事件，当时间到了时，将会调用相应的事件。
*            ：
*    参数    ： taskID ：哪一个任务的定时器
*            ： event_id ：定时器将会产生的事件
*            ： timeout_value ：定时器的毫秒数
*   返回值   ： SUCCESS, 或者 NO_TIMER_AVAIL.
******************************************************************************/
uint8 osal_start_timerEx( uint8 taskID, uint16 event_id, uint16 timeout_value );

