周期采集发送有无人信息
void SampleApp_Send_P2P_Message( void )
{

  unsigned char data = 0+'0';
  if(DATA_PIN == 1)
  { 
        MicroWait(10000); //等待10ms
        if(DATA_PIN == 1)
        { 
             data = 1+'0';
        }
  }
  
  if(data == 0+'0')
  {        
       HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
       HalLcdWriteCH(0,3,5); //无人
       HalLcdWriteCH(1,3,7);
  }
  else
  {
       HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
       HalLcdWriteCH(0,3,4); //有人
       HalLcdWriteCH(1,3,7);
  }
  //调用AF_DataRequest将数据无线广播出去
  if( AF_DataRequest( &SampleApp_P2P_DstAddr,//发送目的地址＋端点地址和传送模式
                       &SampleApp_epDesc,//源(答复或确认)终端的描述（比如操作系统中任务ID等）源EP
                       SAMPLEAPP_P2P_CLUSTERID, //被Profile指定的有效的集群号
                       1,       // 发送数据长度
                       &data,// 发送数据缓冲区
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


协调器得到数据，将相应的数据转发到PC
  case SAMPLEAPP_P2P_CLUSTERID: //收到广播数据  
      if(pkt->cmd.Data[0] == 1+'0')
      {
        HalUARTWrite(0,"有人",4);//输出接收到的
      }
      else
      {
        HalUARTWrite(0,"无人",4);//输出接收到的
      }
      
      HalUARTWrite(0,"\n",1);  //回车换行
      break;