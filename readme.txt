���������غú�Э������·�������ն˺���ԣ������Ӵ��ڽ����յ����ݡ�UartInit OK��
�ַ�����

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