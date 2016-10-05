����һ���汾�еĶ�ʱ�㲥�ĺ����з��͵����¶ȵ���Ϣ��
���¶�ģ��DS18B20���ļ�����������AppĿ¼��
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
  byte str[5];
  char strTemp[10];
  byte temp;
  
  temp = getDs18B20();  //��ȡ�¶�����
  
  str[0] = temp/10+'0';
  str[1] = temp%10+'0';
  str[2] = '\'';
  str[3] = 'C';
  str[4] = '\0';
  
  HalUARTWrite(0,"TEMP:",5);  //�ն˴��������ʾ��Ϣ
  HalUARTWrite(0,str,2);
  HalUARTWrite(0,"\n",1);
  
  osal_memcpy(strTemp,"TEMP:",5);
  osal_memcpy(&strTemp[5],str,5);
  HalLcdWriteString(strTemp,HAL_LCD_LINE_3);  //LCD��ʾ
  // ����AF_DataRequest���������߹㲥��ȥ
  if( AF_DataRequest( &SampleApp_P2P_DstAddr,//����Ŀ�ĵ�ַ���˵��ַ�ʹ���ģʽ
                       &SampleApp_epDesc,//Դ(�𸴻�ȷ��)�ն˵��������������ϵͳ������ID�ȣ�ԴEP
                       SAMPLEAPP_P2P_CLUSTERID, //��Profileָ������Ч�ļ�Ⱥ��
                       4,       // �������ݳ���
                       str,// �������ݻ�����
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