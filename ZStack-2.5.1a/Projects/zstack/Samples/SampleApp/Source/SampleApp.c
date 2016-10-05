/*****************************************************************************
*  文件名称  ： 基于SampleApp串口无线控制LED灯
*    作者    ： 郑朋桥
*    时间    ： 2016/8/20
******************************************************************************/
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "SampleApp.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"


/*********************************************************************
 * GLOBAL VARIABLES
 */

// This list should be filled with Application specific Cluster IDs.
const cId_t SampleApp_ClusterList[SAMPLEAPP_MAX_CLUSTERS] =
{
  SAMPLEAPP_CLUSTERID,
};

const SimpleDescriptionFormat_t SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT,              //  int Endpoint;
  SAMPLEAPP_PROFID,                //  uint16 AppProfId[2];
  SAMPLEAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SAMPLEAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SAMPLEAPP_FLAGS,                 //  int   AppFlags:4;
  SAMPLEAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList,  //  byte *pAppInClusterList;
  SAMPLEAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList   //  byte *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in SampleApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t SampleApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
byte SampleApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // SampleApp_Init() is called.
devStates_t SampleApp_NwkState;


byte SampleApp_TransID;  // This is the unique message ID (counter)

afAddrType_t SampleApp_DstAddr;
#define MY_DEFINE_UART_PORT 0  //自定义串口号（0，1）
#define RX_MAX_LENGTH 20       //接收缓冲区最大值：20个字节
uint8 RX_BUFFER[RX_MAX_LENGTH]; //接收缓冲区


/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SampleApp_HandleKeys( byte shift, byte keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendTheMessage( void );

void UartCallBackFunction(uint8 port, uint8 event); //回调函数声明，定义在最后面

/*   配置串口      */
halUARTCfg_t uartConfig; //定义串口配置结构体变量；
void Uart_Config(void); //函数声明；


/*****************************************************************************
*  函数名称  ： Uart_Config
*  函数介绍  ：配置串口
*            ：
*    参数    ： 无
*   返回值   ： 无
******************************************************************************/

void Uart_Config(void) 
{
  uartConfig.configured = TRUE; //允许配置
  uartConfig.baudRate = HAL_UART_BR_115200; //波特率
  uartConfig.flowControl = FALSE;
  uartConfig.flowControlThreshold = 64;
  uartConfig.rx.maxBufSize = 128;
  uartConfig.tx.maxBufSize = 218;
  uartConfig.idleTimeout = 6;
  uartConfig.intEnable = TRUE;
  uartConfig.callBackFunc = UartCallBackFunction;
}


/******************************************
*              
*     函数名称：SampleApp_Init
*     函数功能：应用层初始化
*
*******************************************/
void SampleApp_Init( uint8 task_id )
{ 
      
    
   
    SampleApp_TaskID = task_id;
    SampleApp_NwkState = DEV_INIT;
    SampleApp_TransID = 0;
  
    // Device hardware initialization can be added here or in main() (Zmain.c).
    // If the hardware is application specific - add it here.
    // If the hardware is other parts of the device add it in main().
  
    SampleApp_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
    SampleApp_DstAddr.endPoint = 0;
    SampleApp_DstAddr.addr.shortAddr = 0;
  
    // Fill out the endpoint description.
    SampleApp_epDesc.endPoint = SAMPLEAPP_ENDPOINT;
    SampleApp_epDesc.task_id = &SampleApp_TaskID;
    SampleApp_epDesc.simpleDesc
              = (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc;
    SampleApp_epDesc.latencyReq = noLatencyReqs;
  
    // Register the endpoint description with the AF
    afRegister( &SampleApp_epDesc );
  

    /*  串口操作  */
    Uart_Config(); //配置串口
    HalUARTOpen(MY_DEFINE_UART_PORT , &uartConfig); //打开串口
    RegisterForKeys( task_id ); // 登记所有的按键事件
    
     
  // Update the display
#if defined ( LCD_SUPPORTED )
    HalLcdWriteString( "SampleApp", HAL_LCD_LINE_1 );
#endif
}

/*********************************************************************
 * @fn      SampleApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
UINT16 SampleApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;

  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {   
        case KEY_CHANGE:
          SampleApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
          }
          break;

        case AF_INCOMING_MSG_CMD:
          SampleApp_MessageMSGCB( MSGpkt );
          break;

        case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          /*if ( (SampleApp_NwkState == DEV_ZB_COORD)
              || (SampleApp_NwkState == DEV_ROUTER)
              || (SampleApp_NwkState == DEV_END_DEVICE) )
          {
            // Start sending "the" message in a regular interval.
            osal_start_timerEx( SampleApp_TaskID,
                                SAMPLEAPP_SEND_MSG_EVT,
                              SAMPLEAPP_SEND_MSG_TIMEOUT );
          }*/
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in SampleApp_Init()).
  if ( events & SAMPLEAPP_SEND_MSG_EVT )
  {
    // Send "the" message
    SampleApp_SendTheMessage();

    // Setup to send message again
    osal_start_timerEx( SampleApp_TaskID,
                        SAMPLEAPP_SEND_MSG_EVT,
                      SAMPLEAPP_SEND_MSG_TIMEOUT );

    // return unprocessed events
    return (events ^ SAMPLEAPP_SEND_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}



/*********************************************************************
 * @fn      SampleApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void SampleApp_HandleKeys( byte shift, byte keys )
{
  // Shift is used to make each button/switch dual purpose.

    if ( keys & HAL_KEY_SW_1 )
    {
      afAddrType_t P2P_DstAddr;
      P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
      P2P_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
      P2P_DstAddr.addr.shortAddr = 0xFFFF; //终端短地址在LCD上有显示，此处换成终端短地址就可以点播了。
    
      if ( AF_DataRequest( &P2P_DstAddr, &SampleApp_epDesc,
                           SAMPLEAPP_CLUSTERID,
                           2,
                           RX_BUFFER,
                           &SampleApp_TransID,
                           AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
      {
        // Successfully requested to be sent.
      }
      else
      {
        // Error occurred in request to send.
      }
    }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{

     
  switch ( pkt->clusterId )
  {
    case SAMPLEAPP_CLUSTERID:
#if defined(ZDO_COORDINATOR)
      // "the" message
  #if defined( LCD_SUPPORTED )
        HalLcdWriteScreen( (char*)pkt->cmd.Data, "rcvd" );
  #elif defined( WIN32 )
        WPRINTSTR( pkt->cmd.Data );
  #endif

#else     
     byte data[2];
     data[0]  = pkt->cmd.Data[0];    //osal_memcpy(&data, pkt->cmd.Data, 1);
     data[1]  = pkt->cmd.Data[1];    //osal_memcpy(&data, pkt->cmd.Data, 1);
     
     if(data[0] == 'd' && data[1] == '1')
       HalLedSet(HAL_LED_1, HAL_LED_MODE_ON); 
     else if(data[0] == 'D' && data[1] == '1')
       HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF); 
     else if(data[0] == 'd' && data[1] == '2')
       HalLedSet(HAL_LED_2, HAL_LED_MODE_ON); 
     else if(data[0] == 'D' && data[1] == '2')
       HalLedSet(HAL_LED_2, HAL_LED_MODE_OFF); 
     else if(data[0] == 'd' && data[1] == '3')
       HalLedSet(HAL_LED_3, HAL_LED_MODE_ON); 
     else if(data[0] == 'D' && data[1] == '3')
       HalLedSet(HAL_LED_3, HAL_LED_MODE_OFF);    
#endif  
     
      break;
  }
}

/*********************************************************************
 * @fn      SampleApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendTheMessage( void )
{
  afAddrType_t P2P_DstAddr;
  P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  P2P_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  P2P_DstAddr.addr.shortAddr = 0xFFFF; //终端短地址在LCD上有显示，此处换成终端短地址就可以点播了。

  if ( AF_DataRequest( &P2P_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_CLUSTERID,
                       2,
                       RX_BUFFER,
                       &SampleApp_TaskID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
    // Successfully requested to be sent.
  }
  else
  {
    // Error occurred in request to send.
  }
}


/*****************************************************************************
*  函数名称  ： UartCallBackFunction
*  函数介绍  ： 串口回调函数
*            ：
*    参数    ： port:串口号
*            ： event:事件
*   返回值   ： 无
******************************************************************************/
static void UartCallBackFunction(uint8 port , uint8 event)
{
  uint8 RX_Length = 0; //接收到字符串大小
  RX_Length = Hal_UART_RxBufLen(MY_DEFINE_UART_PORT); //读取接收字符串大小；
  if(RX_Length != 0) //有数据存在
  {
            //读取串口数据；
        HalUARTRead(MY_DEFINE_UART_PORT , RX_BUFFER , RX_Length);
        SampleApp_SendTheMessage();
        //发送回给电脑,使用 hal_uart.h 的接口函数：
        HalUARTWrite(MY_DEFINE_UART_PORT ,  RX_BUFFER , RX_Length);
        
        HalUARTWrite(MY_DEFINE_UART_PORT ,  "\n" , 1);
  }
}

