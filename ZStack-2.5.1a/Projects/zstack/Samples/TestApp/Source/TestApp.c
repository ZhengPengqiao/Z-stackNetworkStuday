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
  PROVIDED �AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

#define DATA_PIN P0_4      //����P0.7��Ϊ�������������

/*****************************************************************************
*  ��������  �� pinInit
*  ��������  �� ��ʼ������������
*            ��
*    ����    �� ��
*   ����ֵ   �� ��
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
uint8 AppTitle[] = "ALD2530 LED"; //Ӧ�ó�������

// This list should be filled with Application specific Cluster IDs.
const cId_t TestApp_ClusterList[TESTAPP_MAX_CLUSTERS] =
{
  TESTAPP_PERIODIC_CLUSTERID,
  TESTAPP_FLASH_CLUSTERID
};

const SimpleDescriptionFormat_t TestApp_SimpleDesc =
{
  TESTAPP_ENDPOINT,              //  �˵��
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
  TestApp_TaskID = task_id;   //osal���������ID�����û���������������ı�
  TestApp_NwkState = DEV_INIT; 
    /*�豸״̬�� ��Ϊ ZDO ���ж���ĳ�ʼ��״̬����ʼ��Ӧ���豸�������ͣ��豸����
      �ĸı䶼Ҫ����һ���¼���ZD O_STATE_CHANGE�����������ΪZDO ״̬�����˸ı䡣
      �����豸��ʼ����ʱ��һ��Ҫ ������ʼ��Ϊʲô״̬��û�С���ô���� Ҫȥ�����
      �����������Ƿ������������߼��� ���ڵ����硣������һ��������⣬���ǵ� NV_R
      ESTORE �����õĺ�NV_RESTORE �� ����Ϣ���ڷ���ʧ�洢���У� ����ô���豸��
      ��� ��ĳ����������ʱ����������״̬�洢�ڷ���ʧ �洢���У���ôʱ��ֻ��Ҫ��
      ��������״ ̬��������Ҫ���½������߼���������.������Ҫ ���� NV_RESTORE ��
      ���塣
    */
  TestApp_TransID = 0;        //��Ϣ����ID������Ϣʱ��˳��֮�֣�
  HalLcdWriteString( "TestApp OK", HAL_LCD_LINE_1 ); 
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().
//------------------------------ ���ô��� --------------------------------------
  MT_UartInit();   //��ʼ������
  MT_UartRegisterTaskID(task_id); //ע�ᴮ������
  HalUARTWrite(0,"UartInit OK\n",sizeof("UartInit OK\n")); //�򴮿ڷ�������
//------------------------------------------------------------------------------
//------------------------------ ����DS18B20 --------------------------------------
  pinInit();            //���������ų�ʼ��
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

//�öε���˼�ǣ����������HOLD_AUTO_START�궨�壬����������оƬ��ʱ�����ͣ����
//���̣�ֻ���ⲿ�����Ժ�Ż�����оƬ����ʵ������Ҫһ����ť���������������̡�  
#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);
#endif

  // Setup for the periodic message's destination address ���÷������ݵķ�ʽ��Ŀ�ĵ�ַѰַģʽ
  // Broadcast to everyone ����ģʽ:�㲥����
  TestApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;//�㲥
  TestApp_Periodic_DstAddr.endPoint = TESTAPP_ENDPOINT; //ָ���˵��
  TestApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;//ָ��Ŀ�������ַΪ�㲥��ַ

  // Setup for the flash command's destination address - Group 1 �鲥����
  TestApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup; //��Ѱַ
  TestApp_Flash_DstAddr.endPoint = TESTAPP_ENDPOINT; //ָ���˵��
  TestApp_Flash_DstAddr.addr.shortAddr = TESTAPP_FLASH_GROUP;//���0x0001

  // ���÷������ݵķ�ʽ��Ŀ�ĵ�ַѰַģʽ
  // ����ģʽ:�㲥����
  TestApp_P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//�㲥
  TestApp_P2P_DstAddr.endPoint = TESTAPP_ENDPOINT; //ָ���˵��
  TestApp_P2P_DstAddr.addr.shortAddr = 0x0000;       //���͸�Э����

  
  // Fill out the endpoint description. ���屾�豸����ͨ�ŵ�APS��˵�������
  TestApp_epDesc.endPoint = TESTAPP_ENDPOINT; //ָ���˵��
  TestApp_epDesc.task_id = &TestApp_TaskID;   //TestApp ������������ID
  TestApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&TestApp_SimpleDesc;//TestApp��������
  TestApp_epDesc.latencyReq = noLatencyReqs;    //��ʱ����

  // ��AF��Ǽ����������Ǽ�endpoint description��AF,Ҫ�Ը�Ӧ�ý��г�ʼ������AF���еǼǣ�
  // ����Ӧ�ò�����ôһ��EP�Ѿ���ͨ����ʹ�ã���ô�²�Ҫ���йظ�Ӧ�õ���Ϣ��Ӧ��
  // Ҫ���²�����Щ���������Զ��õ��²����ϡ�
  afRegister( &TestApp_epDesc );    //��AF��Ǽ�������

  // Register for all key events - This app will handle all key events
  RegisterForKeys( TestApp_TaskID ); // �Ǽ����еİ����¼�

  // By default, all devices start out in Group 1
  TestApp_Group.ID = TESTAPP_FLASH_GROUP;//���
  osal_memcpy( TestApp_Group.name, "Group 1", 7  );//�趨����
  aps_AddGroup( TESTAPP_ENDPOINT, &TestApp_Group );//�Ѹ���Ǽ���ӵ�APS��
 
#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "TestApp", HAL_LCD_LINE_1 ); //���֧��LCD����ʾ��ʾ��Ϣ
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
//�û�Ӧ��������¼�������
uint16 TestApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG ) //����ϵͳ��Ϣ�ٽ����ж�
  {
    //�������ڱ�Ӧ������TestApp����Ϣ����TestApp_TaskID���
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( TestApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        // Received when a key is pressed
        case KEY_CHANGE://�����¼�
          TestApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD://���������¼�,���ú���AF_DataRequest()��������
          TestApp_MessageMSGCB( MSGpkt );//���ûص��������յ������ݽ��д���
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          //ֻҪ����״̬�����ı䣬��ͨ��ZDO_STATE_CHANGE�¼�֪ͨ���е�����
          //ͬʱ��ɶ�Э������·�������ն˵�����
          TestApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          //if ( (TestApp_NwkState == DEV_ZB_COORD)//ʵ����Э����ֻ������������ȡ�������¼�
          if ( (TestApp_NwkState == DEV_ROUTER) || (TestApp_NwkState == DEV_END_DEVICE) )
          {
            //�����Ŀ��û��ʹ�����ں���
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

      // Release the memory �¼��������ˣ��ͷ���Ϣռ�õ��ڴ�
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available ָ��ָ����һ�����ڻ������Ĵ�������¼���
      //����while ( MSGpkt )���´����¼���ֱ��������û�еȴ������¼�Ϊֹ
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( TestApp_TaskID );
    }

    // return unprocessed events ����δ������¼�
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in TestApp_Init()).
  if ( events & TESTAPP_SEND_P2P_MSG_EVT )
  {
    // �����������¼���
    //TestApp_Send_P2P_Message()�����굱ǰ���������¼���Ȼ��������ʱ��
    //������һ�����������飬����һ��ѭ����ȥ��Ҳ��������˵���������¼��ˣ�
    //������Ϊ��������ʱ�ɼ����ϴ�����
   
    TestApp_Send_P2P_Message();
    // Setup to send message again in normal period (+ a little jitter)
    osal_start_timerEx( TestApp_TaskID, TESTAPP_SEND_P2P_MSG_EVT,
        (TESTAPP_SEND_P2P_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

    // return unprocessed events ����δ������¼�
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
void TestApp_HandleKeys( uint8 shift, uint8 keys ) //��ʵ��û���õ��������ٷ���
{
  (void)shift;  // Intentionally unreferenced parameter
  
  if ( keys & HAL_KEY_SW_6 )
  {
    /* This key sends the Flash Command is sent to Group 1.
     * This device will not receive the Flash Command from this
     * device (even if it belongs to group 1).
     */
     #if defined(ZDO_COORDINATOR)  //Э����ֻ��������
     
     #else                         //·�������ն˲Żᷢ������
        TestApp_SendFlashMessage(0); //���鲥��ʽ��������
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
//�������ݣ�����Ϊ���յ�������
void TestApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  uint8 data;

  switch ( pkt->clusterId ) //�жϴ�ID
  {
    case TESTAPP_P2P_CLUSTERID: //�յ��㲥����  
      if(pkt->cmd.Data[0] == 1+'0')
      {
        HalUARTWrite(0,"����",4);//������յ���
      }
      else
      {
        HalUARTWrite(0,"����",4);//������յ���
      }
      
      HalUARTWrite(0,"\n",1);  //�س�����
      break;

    case TESTAPP_FLASH_CLUSTERID: //�յ��鲥����
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
//��������������Ϣ
void TestApp_Send_P2P_Message( void )
{

  unsigned char data = 0+'0';
  if(DATA_PIN == 1)
  { 
        MicroWait(10000); //�ȴ�10ms
        if(DATA_PIN == 1)
        { 
             data = 1+'0';
        }
  }
  
  if(data == 0+'0')
  {        
       HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
       HalLcdWriteCH(0,3,5); //����
       HalLcdWriteCH(1,3,7);
  }
  else
  {
       HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
       HalLcdWriteCH(0,3,4); //����
       HalLcdWriteCH(1,3,7);
  }
  //����AF_DataRequest���������߹㲥��ȥ
  if( AF_DataRequest( &TestApp_P2P_DstAddr,//����Ŀ�ĵ�ַ���˵��ַ�ʹ���ģʽ
                       &TestApp_epDesc,//Դ(�𸴻�ȷ��)�ն˵��������������ϵͳ������ID�ȣ�ԴEP
                       TESTAPP_P2P_CLUSTERID, //��Profileָ������Ч�ļ�Ⱥ��
                       1,       // �������ݳ���
                       &data,// �������ݻ�����
                       &TestApp_TransID,     // ����ID��
                       AF_DISCV_ROUTE,      // ��Чλ����ķ���ѡ��
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )  //����������ͨ������ΪAF_DEFAULT_RADIUS
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
  // ����AF_DataRequest���������߹㲥��ȥ
  if( AF_DataRequest( &TestApp_Periodic_DstAddr,//����Ŀ�ĵ�ַ���˵��ַ�ʹ���ģʽ
                       &TestApp_epDesc,//Դ(�𸴻�ȷ��)�ն˵��������������ϵͳ������ID�ȣ�ԴEP
                       TESTAPP_PERIODIC_CLUSTERID, //��Profileָ������Ч�ļ�Ⱥ��
                       10,       // �������ݳ���
                       SendData,// �������ݻ�����
                       &TestApp_TransID,     // ����ID��
                       AF_DISCV_ROUTE,      // ��Чλ����ķ���ѡ��
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )  //����������ͨ������ΪAF_DEFAULT_RADIUS
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
void TestApp_SendFlashMessage( uint16 flashTime ) //��ʵ��û���õ��������ٷ���
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
