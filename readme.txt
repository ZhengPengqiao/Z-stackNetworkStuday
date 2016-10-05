�����ڷ��͵�Ե����ݣ�
��ȡ��ʪ�ȣ������ַ����������ַ�����
Э����ֻ�����ַ����������ַ���ͨ������ת����PC��
void SampleApp_Send_P2P_Message( void )
{

    uint8 strData[20];
    uint8 temp[3]; 
    uint8 humidity[3];   

    
    DHT11();             //��ȡ��ʪ��

    //����ʪ�ȵ�ת�����ַ���
    temp[0]=wendu_shi+0x30;
    temp[1]=wendu_ge+0x30;
    humidity[0]=shidu_shi+0x30;
    humidity[1]=shidu_ge+0x30;
    
    osal_memcpy(strData,"TEMP:",5);
    osal_memcpy(&strData[5],temp,2);
    osal_memcpy(&strData[7],"   ",3);
    osal_memcpy(&strData[10],"Hum:",4);
    osal_memcpy(&strData[14],humidity,2);
    strData[16] = (uint8)'\n';
    HalUARTWrite(0,strData, 16);
    HalUARTWrite(0,"\n", 1);

    HalLcdWriteString((char*)strData,HAL_LCD_LINE_3);  //LCD��ʾ
    LCD_P16x16Ch(16,6,0);
    LCD_P16x16Ch(32,6,1);
    LCD_P16x16Ch(48,6,2);
    LCD_P16x16Ch(64,6,3);
    LCD_P16x16Ch(80,6,1);
    LCD_P16x16Ch(96,6,2);
    
  //����AF_DataRequest���������߹㲥��ȥ
  if( AF_DataRequest( &SampleApp_P2P_DstAddr,//����Ŀ�ĵ�ַ���˵��ַ�ʹ���ģʽ
                       &SampleApp_epDesc,//Դ(�𸴻�ȷ��)�ն˵��������������ϵͳ������ID�ȣ�ԴEP
                       SAMPLEAPP_P2P_CLUSTERID, //��Profileָ������Ч�ļ�Ⱥ��
                       16,       // �������ݳ���
                       strData,// �������ݻ�����
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