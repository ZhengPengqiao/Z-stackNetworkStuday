�⹤��ֻ������Zigbee�ṩ�Ľӿ���ʵ�ְ�������LED�ơ�
������S1����ʱ������תLed1С�ơ�



Ҫ�ڳ�ʼ��ʱע�ᰴ���¼�����������ϵͳ�Ż����а����¼�ʱ֪ͨ���ǡ�
/******************************************
*              
*     �������ƣ�SampleApp_Init
*     �������ܣ�Ӧ�ò��ʼ��
*
*******************************************/
void SampleApp_Init( uint8 task_id )
{ 
    RegisterForKeys( task_id ); // �Ǽ����еİ����¼�
}



/******************************************
*              
*     �������ƣ�SampleApp_HandleKeys
*     �������ܣ������¼�������
*
*******************************************/
void SampleApp_HandleKeys( uint8 shift, uint8 keys ) 
{
      (void)shift;  // Intentionally unreferenced parameter
      
      if ( keys & HAL_KEY_SW_6 ) //S1
      {
              HalLedSet(HAL_LED_1,HAL_LED_MODE_TOGGLE); //��תС��
      }
}
