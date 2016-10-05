将程序下载好后（协调器，路由器，终端后可以），连接串口将会收到数据“UartInit OK”
字符串。

在左边workspace目录下比较重要的两个文件夹分别是Zmain和App。我们开发主要在App文件
夹进行，这也是用户自己添加自己代码的地方。主要修改SampleApp.c和SampleApp.h即可，
如果增加传感器则增加相应的模块驱动到App里面，在SampleApp.c中调用就行。

SampleApp.c文件SampleApp_Init()函数中;
//------------------------------ 配置串口 --------------------------------------
  MT_UartInit();   //初始化串口
  MT_UartRegisterTaskID(task_id); //注册串口任务
  HalUARTWrite(0,"Hello world\n",sizeof("Hello world\n")); //向串口发送数据
//------------------------------------------------------------------------------

MT_UART.c文件中MT_UartInit()函数中
 //设置波特率
 uartConfig.baudRate             = MT_UART_DEFAULT_BAUDRATE;
 //设置是否有流控制
 uartConfig.flowControl          = MT_UART_DEFAULT_OVERFLOW;
 
MT_UART.h文件中修改MT_UART_DEFAULT_BAUDRATE、MT_UART_DEFAULT_OVERFLOW来修改波特率
和流控制
#if !defined( MT_UART_DEFAULT_OVERFLOW )
  #define MT_UART_DEFAULT_OVERFLOW       FALSE
#endif

#if !defined MT_UART_DEFAULT_BAUDRATE
#define MT_UART_DEFAULT_BAUDRATE         HAL_UART_BR_115200
#endif
  
  
    //读取串口数据
    HalUARTRead(0,ch,4);