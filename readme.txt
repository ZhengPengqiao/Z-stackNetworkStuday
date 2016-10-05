实验中，只会在终端或者路由联网状态变化时，开启定时任务，协调器不会开启定时任务，
也就不会调用点播函数     
     case ZDO_STATE_CHANGE:
          //只要网络状态发生改变，就通过ZDO_STATE_CHANGE事件通知所有的任务。
          //同时完成对协调器，路由器，终端的设置
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          //if ( (SampleApp_NwkState == DEV_ZB_COORD)//实验中协调器只接收数据所以取消发送事件
          if ( (SampleApp_NwkState == DEV_ROUTER) || (SampleApp_NwkState == DEV_END_DEVICE) )
          {
            //这个项目中没有使用周期函数
            osal_start_timerEx( SampleApp_TaskID,
                              SAMPLEAPP_SEND_P2P_MSG_EVT,
                              SAMPLEAPP_SEND_P2P_MSG_TIMEOUT );
          }
          else
          {
            // Device is no longer in the network
          }
          break;


终端点播周期发送函数，发送当前是否有光

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

  unsigned char data;
  if(DATA_PIN == 1)
  {        
       data = 0+'0';
       HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
       HalLcdWriteCH(0,3,5); //无光
       HalLcdWriteCH(1,3,6);
  }
  else
  {
       data = 1+'0';
       HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
       HalLcdWriteCH(0,3,4); //有光
       HalLcdWriteCH(1,3,6);
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


协调器在接收到终端发送过来的数据时，处理数据并从串口打印信息
  switch ( pkt->clusterId ) //判断簇ID
  {
    case SAMPLEAPP_P2P_CLUSTERID: //收到广播数据  
      if(pkt->cmd.Data[0] == 1+'0')
      {
        HalUARTWrite(0,"有光",4);//输出接收到的
      }
      else
      {
        HalUARTWrite(0,"无光",4);//输出接收到的
      }
      HalUARTWrite(0,"\n",1);  //回车换行
      break;
