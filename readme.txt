在上一个版本中的定时点播的函数中发送的是温度的信息。
将温度模块DS18B20的文件拷贝到工程App目录下
/*********************************************************************
 * @fn      SampleApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */
//分析发送周期信息
void SampleApp_Send_P2P_Message( void )
{
  byte str[5];
  char strTemp[10];
  byte temp;
  
  temp = getDs18B20();  //读取温度数据
  
  str[0] = temp/10+'0';
  str[1] = temp%10+'0';
  str[2] = '\'';
  str[3] = 'C';
  str[4] = '\0';
  
  HalUARTWrite(0,"TEMP:",5);  //终端串口输出提示信息
  HalUARTWrite(0,str,2);
  HalUARTWrite(0,"\n",1);
  
  osal_memcpy(strTemp,"TEMP:",5);
  osal_memcpy(&strTemp[5],str,5);
  HalLcdWriteString(strTemp,HAL_LCD_LINE_3);  //LCD显示
  // 调用AF_DataRequest将数据无线广播出去
  if( AF_DataRequest( &SampleApp_P2P_DstAddr,//发送目的地址＋端点地址和传送模式
                       &SampleApp_epDesc,//源(答复或确认)终端的描述（比如操作系统中任务ID等）源EP
                       SAMPLEAPP_P2P_CLUSTERID, //被Profile指定的有效的集群号
                       4,       // 发送数据长度
                       str,// 发送数据缓冲区
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