ʵ���У�ֻ�����ն˻���·������״̬�仯ʱ��������ʱ����Э�������Ὺ����ʱ����
Ҳ�Ͳ�����õ㲥����     
     case ZDO_STATE_CHANGE:
          //ֻҪ����״̬�����ı䣬��ͨ��ZDO_STATE_CHANGE�¼�֪ͨ���е�����
          //ͬʱ��ɶ�Э������·�������ն˵�����
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          //if ( (SampleApp_NwkState == DEV_ZB_COORD)//ʵ����Э����ֻ������������ȡ�������¼�
          if ( (SampleApp_NwkState == DEV_ROUTER) || (SampleApp_NwkState == DEV_END_DEVICE) )
          {
            //�����Ŀ��û��ʹ�����ں���
            osal_start_timerEx( SampleApp_TaskID,
                              SAMPLEAPP_SEND_P2P_MSG_EVT,
                              SAMPLEAPP_SEND_P2P_MSG_TIMEOUT );
          }
          else
          {
            // Device is no longer in the network
          }
          break;


�ն˵㲥���ڷ��ͺ��������͵�ǰ�Ƿ��й�

/*********************************************************************
 * @fn      SampleApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */
//��������������Ϣ
void SampleApp_Send_P2P_Message( void )
{

  unsigned char data;
  if(DATA_PIN == 1)
  {        
       data = 0+'0';
       HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
       HalLcdWriteCH(0,3,5); //�޹�
       HalLcdWriteCH(1,3,6);
  }
  else
  {
       data = 1+'0';
       HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
       HalLcdWriteCH(0,3,4); //�й�
       HalLcdWriteCH(1,3,6);
  }
  //����AF_DataRequest���������߹㲥��ȥ
  if( AF_DataRequest( &SampleApp_P2P_DstAddr,//����Ŀ�ĵ�ַ���˵��ַ�ʹ���ģʽ
                       &SampleApp_epDesc,//Դ(�𸴻�ȷ��)�ն˵��������������ϵͳ������ID�ȣ�ԴEP
                       SAMPLEAPP_P2P_CLUSTERID, //��Profileָ������Ч�ļ�Ⱥ��
                       1,       // �������ݳ���
                       &data,// �������ݻ�����
                       &SampleApp_TransID,     // ����ID��
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


Э�����ڽ��յ��ն˷��͹���������ʱ���������ݲ��Ӵ��ڴ�ӡ��Ϣ
  switch ( pkt->clusterId ) //�жϴ�ID
  {
    case SAMPLEAPP_P2P_CLUSTERID: //�յ��㲥����  
      if(pkt->cmd.Data[0] == 1+'0')
      {
        HalUARTWrite(0,"�й�",4);//������յ���
      }
      else
      {
        HalUARTWrite(0,"�޹�",4);//������յ���
      }
      HalUARTWrite(0,"\n",1);  //�س�����
      break;
