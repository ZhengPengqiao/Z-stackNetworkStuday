在网络建立的时候执行定时函数，指向事件SAMPLEAPP_SEND_P2P_MSG_EVT
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
处理定时事件，调用函数SampleApp_Send_P2P_Message发送数据。
  if ( events & SAMPLEAPP_SEND_P2P_MSG_EVT )
  {
    // 处理周期性事件，
    //SampleApp_Send_P2P_Message()处理完当前的周期性事件，然后启动定时器
    //开启下一个周期性事情，这样一种循环下去，也即是上面说的周期性事件了，
    //可以做为传感器定时采集、上传任务
   
    SampleApp_Send_P2P_Message();
    // Setup to send message again in normal period (+ a little jitter)
    osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_SEND_P2P_MSG_EVT,
        (SAMPLEAPP_SEND_P2P_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

    // return unprocessed events 返回未处理的事件
    return (events ^ SAMPLEAPP_SEND_P2P_MSG_EVT);
  }
  

点播方式发送数据
void SampleApp_Send_P2P_Message( void )
{
  byte SendData[11]="1234567890";

  // 调用AF_DataRequest将数据无线广播出去
  if( AF_DataRequest( &SampleApp_P2P_DstAddr,//发送目的地址＋端点地址和传送模式
                       &SampleApp_epDesc,//源(答复或确认)终端的描述（比如操作系统中任务ID等）源EP
                       SAMPLEAPP_P2P_CLUSTERID, //被Profile指定的有效的集群号
                       10,       // 发送数据长度
                       SendData,// 发送数据缓冲区
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
发送地址位协调器
// 设置发送数据的方式和目的地址寻址模式
// 发送模式:点播发送
SampleApp_P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;//点播
SampleApp_P2P_DstAddr.endPoint = SAMPLEAPP_ENDPOINT; //指定端点号
SampleApp_P2P_DstAddr.addr.shortAddr = 0x0000;       //发送给协调器


协调器接收并通过串口转发
    case SAMPLEAPP_P2P_CLUSTERID: //收到广播数据
      HalUARTWrite(0,"RX:",3); //提示信息
      HalUARTWrite(0,pkt->cmd.Data,pkt->cmd.DataLength);//输出接收到的
      HalUARTWrite(0,"\n",1);  //回车换行
      break;
      
      
################################################################################