在周期发送点对点数据；
读取温湿度，构造字符串，发送字符串；
协调器只接收字符串，并将字符串通过串口转发到PC机
void SampleApp_Send_P2P_Message( void )
{

    uint8 strData[20];
    uint8 temp[3]; 
    uint8 humidity[3];   

    
    DHT11();             //获取温湿度

    //将温湿度的转换成字符串
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

    HalLcdWriteString((char*)strData,HAL_LCD_LINE_3);  //LCD显示
    LCD_P16x16Ch(16,6,0);
    LCD_P16x16Ch(32,6,1);
    LCD_P16x16Ch(48,6,2);
    LCD_P16x16Ch(64,6,3);
    LCD_P16x16Ch(80,6,1);
    LCD_P16x16Ch(96,6,2);
    
  //调用AF_DataRequest将数据无线广播出去
  if( AF_DataRequest( &SampleApp_P2P_DstAddr,//发送目的地址＋端点地址和传送模式
                       &SampleApp_epDesc,//源(答复或确认)终端的描述（比如操作系统中任务ID等）源EP
                       SAMPLEAPP_P2P_CLUSTERID, //被Profile指定的有效的集群号
                       16,       // 发送数据长度
                       strData,// 发送数据缓冲区
                       &SampleApp_TransID,     // 任务ID号
                       AF_DISCV_ROUTE,      // 有效位掩码的发送选项
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )  //传送跳数，通常设置为AF_DEFAULT_RADIUS
  {
  }
  else
  {
    HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
    // Error occurred in request to send.
  }
 
}