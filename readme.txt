
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

case SAMPLEAPP_P2P_CLUSTERID: //�յ��㲥����  
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