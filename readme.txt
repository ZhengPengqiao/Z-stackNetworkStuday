�����罨����ʱ��ִ�ж�ʱ������ָ���¼�SAMPLEAPP_SEND_P2P_MSG_EVT
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
����ʱ�¼������ú���SampleApp_Send_P2P_Message�������ݡ�
  if ( events & SAMPLEAPP_SEND_P2P_MSG_EVT )
  {
    // �����������¼���
    //SampleApp_Send_P2P_Message()�����굱ǰ���������¼���Ȼ��������ʱ��
    //������һ�����������飬����һ��ѭ����ȥ��Ҳ��������˵���������¼��ˣ�
    //������Ϊ��������ʱ�ɼ����ϴ�����
   
    SampleApp_Send_P2P_Message();
    // Setup to send message again in normal period (+ a little jitter)
    osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_SEND_P2P_MSG_EVT,
        (SAMPLEAPP_SEND_P2P_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

    // return unprocessed events ����δ������¼�
    return (events ^ SAMPLEAPP_SEND_P2P_MSG_EVT);
  }
  

�㲥��ʽ��������
void SampleApp_Send_P2P_Message( void )
{
  byte SendData[11]="1234567890";

  // ����AF_DataRequest���������߹㲥��ȥ
  if( AF_DataRequest( &SampleApp_P2P_DstAddr,//����Ŀ�ĵ�ַ���˵��ַ�ʹ���ģʽ
                       &SampleApp_epDesc,//Դ(�𸴻�ȷ��)�ն˵��������������ϵͳ������ID�ȣ�ԴEP
                       SAMPLEAPP_P2P_CLUSTERID, //��Profileָ������Ч�ļ�Ⱥ��
                       10,       // �������ݳ���
                       SendData,// �������ݻ�����
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
���͵�ַλЭ����
// ���÷������ݵķ�ʽ��Ŀ�ĵ�ַѰַģʽ
// ����ģʽ:�㲥����
SampleApp_P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//�㲥
SampleApp_P2P_DstAddr.endPoint = SAMPLEAPP_ENDPOINT; //ָ���˵��
SampleApp_P2P_DstAddr.addr.shortAddr = 0x0000;       //���͸�Э����


Э�������ղ�ͨ������ת��
    case SAMPLEAPP_P2P_CLUSTERID: //�յ��㲥����
      HalUARTWrite(0,"RX:",3); //��ʾ��Ϣ
      HalUARTWrite(0,pkt->cmd.Data,pkt->cmd.DataLength);//������յ���
      HalUARTWrite(0,"\n",1);  //�س�����
      break;
      
      
################################################################################