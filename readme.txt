�ն�ͨ��  �����¼������鲥���ݣ�Э����ֻ����
  if ( keys & HAL_KEY_SW_6 )
  {
    /* This key sends the Flash Command is sent to Group 1.
     * This device will not receive the Flash Command from this
     * device (even if it belongs to group 1).
     */
     #if defined(ZDO_COORDINATOR)  //Э����ֻ��������
     
     #else                         //·�������ն˲Żᷢ������
        SampleApp_SendFlashMessage(0); //���鲥��ʽ��������
     #endif
  }


�鲥���ͺ�����С��״̬��תLedState��Ȼ���͸�Э����
void SampleApp_SendFlashMessage( uint16 flashTime ) //��ʵ��û���õ��������ٷ���
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


Э�����ڽ��յ�����ʱ���жϽ��յ����ݲ�������Ӧ��״̬
    case SAMPLEAPP_FLASH_CLUSTERID: //�յ��鲥����
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


�����workspaceĿ¼�±Ƚ���Ҫ�������ļ��зֱ���Zmain��App�����ǿ�����Ҫ��App�ļ�
�н��У���Ҳ���û��Լ�����Լ�����ĵط�����Ҫ�޸�SampleApp.c��SampleApp.h���ɣ�
������Ӵ�������������Ӧ��ģ��������App���棬��SampleApp.c�е��þ��С�

SampleApp.c�ļ�SampleApp_Init()������;
//------------------------------ ���ô��� --------------------------------------
  MT_UartInit();   //��ʼ������
  MT_UartRegisterTaskID(task_id); //ע�ᴮ������
  HalUARTWrite(0,"Hello world\n",sizeof("Hello world\n")); //�򴮿ڷ�������
//------------------------------------------------------------------------------

MT_UART.c�ļ���MT_UartInit()������
 //���ò�����
 uartConfig.baudRate             = MT_UART_DEFAULT_BAUDRATE;
 //�����Ƿ���������
 uartConfig.flowControl          = MT_UART_DEFAULT_OVERFLOW;
 
MT_UART.h�ļ����޸�MT_UART_DEFAULT_BAUDRATE��MT_UART_DEFAULT_OVERFLOW���޸Ĳ�����
��������
#if !defined( MT_UART_DEFAULT_OVERFLOW )
  #define MT_UART_DEFAULT_OVERFLOW       FALSE
#endif

#if !defined MT_UART_DEFAULT_BAUDRATE
#define MT_UART_DEFAULT_BAUDRATE         HAL_UART_BR_115200
#endif
  
  
    //��ȡ��������
    HalUARTRead(0,ch,4);
    
    

/*****************************************************************************
* ʵ����Э����ֻ������������ȡ�������¼���ZDO_STATE_CHANGE����������
******************************************************************************/

case ZDO_STATE_CHANGE:
//ֻҪ����״̬�����ı䣬��ͨ��ZDO_STATE_CHANGE�¼�֪ͨ���е�����
//ͬʱ��ɶ�Э������·�������ն˵�����
SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
//if ( (SampleApp_NwkState == DEV_ZB_COORD)//ʵ����Э����ֻ������������ȡ�������¼�
if ( (SampleApp_NwkState == DEV_ROUTER) || (SampleApp_NwkState == DEV_END_DEVICE) )
{
  // Start sending the periodic message in a regular interval.
  //�����ʱ��ֻ��Ϊ����������Ϣ�����ģ��豸������ʼ��������￪ʼ
  //������һ��������Ϣ�ķ��ͣ�Ȼ���ܶ���ʼ��ȥ
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
*  ��������  �� osal_start_timerEx
*  ��������  �� ��������ṩ��ʱ�¼�����ʱ�䵽��ʱ�����������Ӧ���¼���
*            ��
*    ����    �� taskID ����һ������Ķ�ʱ��
*            �� event_id ����ʱ������������¼�
*            �� timeout_value ����ʱ���ĺ�����
*   ����ֵ   �� SUCCESS, ���� NO_TIMER_AVAIL.
******************************************************************************/
uint8 osal_start_timerEx( uint8 taskID, uint16 event_id, uint16 timeout_value );

