/**************************************************************************************************
  Filename:       TestApp.c
  Revised:        $Date: 2009-03-18 15:56:27 -0700 (Wed, 18 Mar 2009) $
  Revision:       $Revision: 19453 $

  Description:    Sample Application (no Profile).


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends it's messages either as broadcast or
  broadcast filtered group messages.  The other (more normal)
  message addressing is unicast.  Most of the other sample
  applications are written to support the unicast message model.

  Key control:
    SW1:  Sends a flash command to all devices in Group 1.
    SW2:  Adds/Removes (toggles) this device in and out
          of Group 1.  This will enable and disable the
          reception of the flash command.
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "TestApp.h"
#include "TestAppHw.h"

#include "OnBoard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "MT_UART.h"
#include "MT_APP.h"
#include "MT.h"


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define DATA_PIN P0_4      //定义P0.7口为传感器的输入端

/*****************************************************************************
*  函数名称  ： pinInit
*  函数介绍  ： 初始化传感器引脚
*            ：
*    参数    ： 无
*   返回值   ： 无
******************************************************************************/
void pinInit()
{
  P0SEL &= ~(1 << 4);
  P0DIR &= ~(1 << 4);
}

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 AppTitle[] = "ALD2530 LED"; //应用程序名称

// This list should be filled with Application specific Cluster IDs.
const cId_t TestApp_ClusterList[TESTAPP_MAX_CLUSTERS] =
{
  TESTAPP_PERIODIC_CLUSTERID,
  TESTAPP_FLASH_CLUSTERID
};

const SimpleDescriptionFormat_t TestApp_SimpleDesc =
{
  TESTAPP_ENDPOINT,              //  端点号
  TESTAPP_PROFID,                //  uint16 AppProfId[2];
  TESTAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  TESTAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  TESTAPP_FLAGS,                 //  int   AppFlags:4;
  TESTAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)TestApp_ClusterList,  //  uint8 *pAppInClusterList;
  TESTAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)TestApp_ClusterList   //  uint8 *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in TestApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t TestApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8 LedState = 0;
uint8 TestApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // TestApp_Init() is called.
devStates_t TestApp_NwkState;

uint8 TestApp_TransID;  // This is the unique message ID (counter)

afAddrType_t TestApp_Periodic_DstAddr;
afAddrType_t TestApp_Flash_DstAddr;
afAddrType_t TestApp_P2P_DstAddr;

aps_Group_t TestApp_Group;

uint8 TestAppPeriodicCounter = 0;
uint8 TestAppFlashCounter = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void TestApp_HandleKeys( uint8 shift, uint8 keys );
void TestApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void TestApp_SendPeriodicMessage( void );
void TestApp_SendFlashMessage( uint16 flashTime );
void TestApp_Send_P2P_Message(void);
/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      TestApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void TestApp_Init( uint8 task_id )
{ 
  TestApp_TaskID = task_id;   //osal分配的任务ID随着用户添加任务的增多而改变
  TestApp_NwkState = DEV_INIT; 
    /*设备状态设 定为 ZDO 层中定义的初始化状态、初始化应用设备网络类型，设备类型
      的改变都要产生一个事件—ZD O_STATE_CHANGE，从字面理解为ZDO 状态发生了改变。
      以在设备初始化的时候一定要 把它初始化为什么状态都没有。那么它就 要去检测整
      个环境，看是否能重新立或者加入 存在的网络。但是有一种情况例外，就是当 NV_R
      ESTORE 被设置的候（NV_RESTORE 是 把信息存在非易失存储器中） ，那么当设备断
      电或 者某种意外重启时，由于网络状态存储在非易失 存储器中，那么时就只需要恢
      复其网络状 态，而不需要重新建立或者加入网络了.这里需要 设置 NV_RESTORE 宏
      定义。
    */
  TestApp_TransID = 0;        //消息发送ID（多消息时有顺序之分）
  HalLcdWriteString( "TestApp OK", HAL_LCD_LINE_1 ); 
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().
//------------------------------ 配置串口 --------------------------------------
  MT_UartInit();   //初始化串口
  MT_UartRegisterTaskID(task_id); //注册串口任务
  HalUARTWrite(0,"UartInit OK\n",sizeof("UartInit OK\n")); //向串口发送数据
//------------------------------------------------------------------------------
//------------------------------ 配置DS18B20 --------------------------------------
  pinInit();            //传感器引脚初始化
//------------------------------------------------------------------------------

  
 #if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in TestAppHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
    zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
  else
    zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
#endif // BUILD_ALL_DEVICES

//该段的意思是，如果设置了HOLD_AUTO_START宏定义，将会在启动芯片的时候会暂停启动
//流程，只有外部触发以后才会启动芯片。其实就是需要一个按钮触发它的启动流程。  
#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);
#endif

  // Setup for the periodic message's destination address 设置发送数据的方式和目的地址寻址模式
  // Broadcast to everyone 发送模式:广播发送
  TestApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;//广播
  TestApp_Periodic_DstAddr.endPoint = TESTAPP_ENDPOINT; //指定端点号
  TestApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;//指定目的网络地址为广播地址

  // Setup for the flash command's destination address - Group 1 组播发送
  TestApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup; //组寻址
  TestApp_Flash_DstAddr.endPoint = TESTAPP_ENDPOINT; //指定端点号
  TestApp_Flash_DstAddr.addr.shortAddr = TESTAPP_FLASH_GROUP;//组号0x0001

  // 设置发送数据的方式和目的地址寻址模式
  // 发送模式:点播发送
  TestApp_P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//点播
  TestApp_P2P_DstAddr.endPoint = TESTAPP_ENDPOINT; //指定端点号
  TestApp_P2P_DstAddr.addr.shortAddr = 0x0000;       //发送给协调器

  
  // Fill out the endpoint description. 定义本设备用来通信的APS层端点描述符
  TestApp_epDesc.endPoint = TESTAPP_ENDPOINT; //指定端点号
  TestApp_epDesc.task_id = &TestApp_TaskID;   //TestApp 描述符的任务ID
  TestApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&TestApp_SimpleDesc;//TestApp简单描述符
  TestApp_epDesc.latencyReq = noLatencyReqs;    //延时策略

  // 向AF层登记描述符，登记endpoint description到AF,要对该应用进行初始化并在AF进行登记，
  // 告诉应用层有这么一个EP已经开通可以使用，那么下层要是有关该应用的信息或应用
  // 要对下层做哪些操作，就自动得到下层的配合。
  afRegister( &TestApp_epDesc );    //向AF层登记描述符

  // Register for all key events - This app will handle all key events
  RegisterForKeys( TestApp_TaskID ); // 登记所有的按键事件

  // By default, all devices start out in Group 1
  TestApp_Group.ID = TESTAPP_FLASH_GROUP;//组号
  osal_memcpy( TestApp_Group.name, "Group 1", 7  );//设定组名
  aps_AddGroup( TESTAPP_ENDPOINT, &TestApp_Group );//把该组登记添加到APS中
 
#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "TestApp", HAL_LCD_LINE_1 ); //如果支持LCD，显示提示信息
#endif
}

/*********************************************************************
 * @fn      TestApp_ProcessEvent
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
//用户应用任务的事件处理函数
uint16 TestApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG ) //接收系统消息再进行判断
  {
    //接收属于本应用任务TestApp的消息，以TestApp_TaskID标记
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( TestApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        // Received when a key is pressed
        case KEY_CHANGE://按键事件
          TestApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD://接收数据事件,调用函数AF_DataRequest()接收数据
          TestApp_MessageMSGCB( MSGpkt );//调用回调函数对收到的数据进行处理
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          //只要网络状态发生改变，就通过ZDO_STATE_CHANGE事件通知所有的任务。
          //同时完成对协调器，路由器，终端的设置
          TestApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          //if ( (TestApp_NwkState == DEV_ZB_COORD)//实验中协调器只接收数据所以取消发送事件
          if ( (TestApp_NwkState == DEV_ROUTER) || (TestApp_NwkState == DEV_END_DEVICE) )
          {
            //这个项目中没有使用周期函数
            osal_start_timerEx( TestApp_TaskID,
                              TESTAPP_SEND_P2P_MSG_EVT,
                              TESTAPP_SEND_P2P_MSG_TIMEOUT );
          }
          else
          {
            // Device is no longer in the network
          }
          break;

        default:
          break;
      }

      // Release the memory 事件处理完了，释放消息占用的内存
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available 指针指向下一个放在缓冲区的待处理的事件，
      //返回while ( MSGpkt )重新处理事件，直到缓冲区没有等待处理事件为止
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( TestApp_TaskID );
    }

    // return unprocessed events 返回未处理的事件
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in TestApp_Init()).
  if ( events & TESTAPP_SEND_P2P_MSG_EVT )
  {
    // 处理周期性事件，
    //TestApp_Send_P2P_Message()处理完当前的周期性事件，然后启动定时器
    //开启下一个周期性事情，这样一种循环下去，也即是上面说的周期性事件了，
    //可以做为传感器定时采集、上传任务
   
    TestApp_Send_P2P_Message();
    // Setup to send message again in normal period (+ a little jitter)
    osal_start_timerEx( TestApp_TaskID, TESTAPP_SEND_P2P_MSG_EVT,
        (TESTAPP_SEND_P2P_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

    // return unprocessed events 返回未处理的事件
    return (events ^ TESTAPP_SEND_P2P_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */
/*********************************************************************
 * @fn      TestApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void TestApp_HandleKeys( uint8 shift, uint8 keys ) //此实验没有用到，后面再分析
{
  (void)shift;  // Intentionally unreferenced parameter
  
  if ( keys & HAL_KEY_SW_6 )
  {
    /* This key sends the Flash Command is sent to Group 1.
     * This device will not receive the Flash Command from this
     * device (even if it belongs to group 1).
     */
     #if defined(ZDO_COORDINATOR)  //协调器只接收数据
     
     #else                         //路由器和终端才会发送数据
        TestApp_SendFlashMessage(0); //以组播方式发送数据
     #endif
  }

  
  if ( keys & HAL_KEY_SW_2 )
  {
    /* The Flashr Command is sent to Group 1.
     * This key toggles this device in and out of group 1.
     * If this device doesn't belong to group 1, this application
     * will not receive the Flash command sent to group 1.
     */
    aps_Group_t *grp;
    grp = aps_FindGroup( TESTAPP_ENDPOINT, TESTAPP_FLASH_GROUP );
    if ( grp )
    {
      // Remove from the group
      aps_RemoveGroup( TESTAPP_ENDPOINT, TESTAPP_FLASH_GROUP );
    }
    else
    {
      // Add to the flash group
      aps_AddGroup( TESTAPP_ENDPOINT, &TestApp_Group );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      TestApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
//接收数据，参数为接收到的数据
void TestApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  uint8 data;

  switch ( pkt->clusterId ) //判断簇ID
  {
    case TESTAPP_P2P_CLUSTERID: //收到广播数据  
      if(pkt->cmd.Data[0] == 1+'0')
      {
        HalUARTWrite(0,"有人",4);//输出接收到的
      }
      else
      {
        HalUARTWrite(0,"无人",4);//输出接收到的
      }
      
      HalUARTWrite(0,"\n",1);  //回车换行
      break;

    case TESTAPP_FLASH_CLUSTERID: //收到组播数据
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
  }
}

/*********************************************************************
 * @fn      TestApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */
//分析发送周期信息
void TestApp_Send_P2P_Message( void )
{

  unsigned char data = 0+'0';
  if(DATA_PIN == 1)
  { 
        MicroWait(10000); //等待10ms
        if(DATA_PIN == 1)
        { 
             data = 1+'0';
        }
  }
  
  if(data == 0+'0')
  {        
       HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
       HalLcdWriteCH(0,3,5); //无人
       HalLcdWriteCH(1,3,7);
  }
  else
  {
       HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
       HalLcdWriteCH(0,3,4); //有人
       HalLcdWriteCH(1,3,7);
  }
  //调用AF_DataRequest将数据无线广播出去
  if( AF_DataRequest( &TestApp_P2P_DstAddr,//发送目的地址＋端点地址和传送模式
                       &TestApp_epDesc,//源(答复或确认)终端的描述（比如操作系统中任务ID等）源EP
                       TESTAPP_P2P_CLUSTERID, //被Profile指定的有效的集群号
                       1,       // 发送数据长度
                       &data,// 发送数据缓冲区
                       &TestApp_TransID,     // 任务ID号
                       AF_DISCV_ROUTE,      // 有效位掩码的发送选项
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )  //传送跳数，通常设置为AF_DEFAULT_RADIUS
  {
  }
  else
  {
    HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
    // Error occurred in request to send.
  }
 
}

void TestApp_SendPeriodicMessage(void)
{
  byte SendData[11]="1234567890";
  // 调用AF_DataRequest将数据无线广播出去
  if( AF_DataRequest( &TestApp_Periodic_DstAddr,//发送目的地址＋端点地址和传送模式
                       &TestApp_epDesc,//源(答复或确认)终端的描述（比如操作系统中任务ID等）源EP
                       TESTAPP_PERIODIC_CLUSTERID, //被Profile指定的有效的集群号
                       10,       // 发送数据长度
                       SendData,// 发送数据缓冲区
                       &TestApp_TransID,     // 任务ID号
                       AF_DISCV_ROUTE,      // 有效位掩码的发送选项
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )  //传送跳数，通常设置为AF_DEFAULT_RADIUS
  {
  }
  else
  {
    HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
    // Error occurred in request to send.
  }
}

/*********************************************************************
 * @fn      TestApp_SendFlashMessage
 *
 * @brief   Send the flash message to group 1.
 *
 * @param   flashTime - in milliseconds
 *
 * @return  none
 */
void TestApp_SendFlashMessage( uint16 flashTime ) //此实验没有用到，后面再分析
{
  LedState = ~LedState;

  if ( AF_DataRequest( &TestApp_Flash_DstAddr, 
                       &TestApp_epDesc,
                       TESTAPP_FLASH_CLUSTERID,
                       1,
                       &LedState,
                       &TestApp_TransID,
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

/*********************************************************************
*********************************************************************/
