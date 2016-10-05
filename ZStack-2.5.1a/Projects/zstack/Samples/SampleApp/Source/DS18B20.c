#include "ds18b20.h" 

#define Ds18b20IO P0_7       //�¶ȴ���������

/*****************************************************************************
*  ��������  �� Delay_us
*  ��������  ��  ʹ��Time1��ʱ��ģģʽ�����о�ȷ��λ
*            ��
*    ����    �� k : ��ʱk us
*   ����ֵ   �� ��
******************************************************************************/

void Delay_us(unsigned int k)//us��ʱ����
{ 

    T1CC0L = 0x10;
    T1CC0H = 0x00;
    T1CCTL0 |= (1 << 2);  //ģģʽֻ��ʹ��0ͨ��
    T1CTL = 0x02; 
    //һ��16M = 16000K = 16000000 ��
    while(k)
    {
        while( ( T1STAT & (1 << 0) ) != 1);
        T1STAT &= ~(1 << 0);
        k--;
    }
    T1CTL = 0x00; //�رն�ʱ��
}

void Ds18b20InputInitial(void)//���ö˿�Ϊ����
{
    P0DIR &= 0x7f;
}

void Ds18b20OutputInitial(void)//���ö˿�Ϊ���
{
    P0DIR |= 0x80;
}

char get18B20Ack()
{
  EA = 0;
  char ret = 1;
  Ds18b20OutputInitial();
  Ds18b20IO = 1;
  Delay_us(50);
  Ds18b20IO = 0;
  Delay_us(750);
  Ds18b20IO = 1;
  Ds18b20InputInitial();
  Delay_us(60);
  ret = Ds18b20IO;
  Ds18b20OutputInitial();
  Ds18b20IO = 1;
  Delay_us(240);
  EA = 1;
  return ret;
  
}


void write18B20(unsigned char ch)
{
  unsigned char i = 0;
  EA = 0;
  Ds18b20OutputInitial();
  for(i = 0; i < 8; i++)
  {
    if(ch & (1 << i))
    {
      Ds18b20IO = 0;
      Delay_us(6);
      Ds18b20IO = 1;
      Delay_us(60);      
    }
    else
    {
      Ds18b20IO = 0;
      Delay_us(60);
      Ds18b20IO = 1;
      Delay_us(6);      
    }
      
  }
  EA = 1;
}

unsigned char read18B20()
{
    unsigned char ret = 0;
    unsigned char i;
    EA = 0;
    Ds18b20OutputInitial();
    Ds18b20IO = 1;
    Delay_us(10);
    for(i = 0; i < 8; i++)
    {
      Ds18b20OutputInitial();
      Ds18b20IO = 0;
      Delay_us(3);
      Ds18b20IO = 1;
      Delay_us(3);
      Ds18b20InputInitial();
      ret |= (Ds18b20IO << i);
      Ds18b20IO = 0;
      Delay_us(30);
    }
    EA = 1;
    return ret;
}


//�¶ȶ�ȡ����
unsigned char getDs18B20(void) 
{
    unsigned char V1,V2;   //����ߵ�8λ ����
    unsigned char temp;    //�����¶Ȼ���Ĵ���
    char status;
    status = get18B20Ack();
    if( status == 0)
    {
        write18B20(0xcc);    // ����������кŵĲ���
        write18B20(0x44);    // �����¶�ת��
    }
    
    status = get18B20Ack();
    if( status == 0)
    {
        write18B20(0xcc);    //����������кŵĲ��� 
        write18B20(0xbe);    //��ȡ�¶ȼĴ����ȣ����ɶ�9���Ĵ����� ǰ���������¶�
    }
    
    V1 = read18B20();    //��λ
    V2 = read18B20();    //��λ
    temp = ((V1 >> 4)+((V2 & 0x07)*16)); //ת������ 

    return temp;
}

//�¶ȶ�ȡ���� ��1λС��λ
float getFloatDs18B20(void) 
{
    unsigned char V1,V2;   //����ߵ�8λ ����
    unsigned int temp;     //�����¶Ȼ���Ĵ���
    char status;
    float fValue;
    status = get18B20Ack();
    if( status == 0)
    {
        write18B20(0xcc);    // ����������кŵĲ���
        write18B20(0x44);    // �����¶�ת��
    }
    
    status = get18B20Ack();
    if( status == 0)
    {
        write18B20(0xcc);    //����������кŵĲ��� 
        write18B20(0xbe);    //��ȡ�¶ȼĴ����ȣ����ɶ�9���Ĵ����� ǰ���������¶�
    }
    V1 = read18B20();    //��λ
    V2 = read18B20();    //��λ
    //temp = ((V1 >> 4)+((V2 & 0x07)*16)); //ת������ 
    temp=V2*0xFF+V1;
    fValue = temp*0.0625;
    
    return fValue;
}
