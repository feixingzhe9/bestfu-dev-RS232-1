/***************************Copyright BestFu 2014-05-14*************************
文 件：  USARTx.c
说 明：  串口寄存器配置文件
编 译：  Keil uVision4 V4.54.0.0
版 本：  v2.0
编 写：  Unarty
日 期：  2014.08.20
修　改：    暂无
*******************************************************************************/
#include "USARTx.h"
#include "GPIO.h"
#include "Sys.h"
#include "fifo.h"
#include "Thread.h"

extern FIFO_t MusicRcveCmdfifo;     //锁接收指令队列


UART_Rec_t UART_Recieve;
Query_Recieve_t QueryRecieve;


u32 Bound_Rate[] = {1200,2400,4800,9600,14400,19200,38400,43000,56000,115200,128000,256000};


u8 isTest[] = "232-test";
u8 testFeedback[] = "232-test-OK";

/*******************************************************************************
函 数 名:    CompareString
功能说明:    比较字符串
参    数:    	u8 *str1,
				u8 *str2,
				u8 len
返 回 值:   	1:相同
				0：不同
*******************************************************************************/
u8 CompareString(u8 *str1,u8 *str2,u8 len)
{
	u8 i;
	if(len != sizeof(isTest)-1)
	{
		return 0;
	}
		
	for(i=0;i<len;i++)
	{
		if(str1[i] != str2[i])
		{
			return 0;
		}		
	}
	return 1;
}
/*******************************************************************************
函 数 名:    UART_RecTimeOutCnt
功能说明:    判断数据是否接收完毕
参    数:    无
返 回 值:    无
*******************************************************************************/
void UART_RecTimeOutCnt(void)
{
    UART_Recieve.timeOutCnt++;
    if (UART_Recieve.timeOutCnt > UART_RECIEVE_TIME_OUT)//超时后的处理
    {
        UART_Recieve.timeOutFlag = TRUE;
        UART_Recieve.timeOutCnt = UART_RECIEVE_TIME_OUT + 1;
			
				UART_Recieve.Len = UART_Recieve.Cnt;
				UART_Recieve.Cnt = 0;
				//UART_Recieve.dataRecieveFlag = FALSE;
			
				Thread_Logout(UART_RecTimeOutCnt);
    }
    else
    {
        UART_Recieve.timeOutFlag = FALSE;
    }

}

/*******************************************************************************
函 数 名:    CheckQueryRecieve
功能说明:   接收查询指令发送后串口传来的数据
参    数:    无
返 回 值:    无
*******************************************************************************/
void CheckQueryRecieve(void)
{
	static u8 cnt = 0;
	u8 i;
	//u8 *temp;
	cnt++;
	if(cnt > QUERY_WAIT_TIME)
	{
		QueryRecieve.flag = FALSE;
		cnt = 0;
		Thread_Logout(CheckQueryRecieve);
		return;
	}
	
	if((TRUE == UART_Recieve.dataRecieveFlag) && (TRUE == UART_Recieve.timeOutFlag))
	{
		QueryRecieve.len = UART_Recieve.Len;
		//temp = UART_Recieve.data;
		for(i=0;i<QueryRecieve.len;i++)
		{
			QueryRecieve.data[i] = UART_Recieve.data[i];//temp;
		}
		
		
		
		if(1 == CompareString(isTest,QueryRecieve.data,QueryRecieve.len))//检查发来的数据是否 串口测试数据
		{
			USARTx_Send(testFeedback,sizeof(testFeedback));
			Clr_QueryRcvData();
		}
		
		
		UART_Recieve.dataRecieveFlag = FALSE;
		UART_Recieve.timeOutFlag = FALSE;
		
//		Thread_Logout(CheckQueryRecieve);
	}
}


/*******************************************************************************
函 数 名:   Clr_QueryRcvData
功能说明:   清除串口接收的数据
参    数:    无
返 回 值:    无
*******************************************************************************/
void Clr_QueryRcvData(void)
{
		u8 i;
		for(i=0;i<200;i++)
		{
			QueryRecieve.data[i] = 0;
		}
		QueryRecieve.cnt = 0;
		QueryRecieve.len = 0;
	//	QueryRecieve.flag = FALSE;		
}
/*******************************************************************************
函 数 名:    Clr_UART_RecTimeOut
功能说明:   清除串口接收超时计数器
参    数:     无
返 回 值:    无
*******************************************************************************/
void Clr_UART_RecTimeOut(void)
{
    UART_Recieve.timeOutCnt = 0;
    UART_Recieve.timeOutFlag = FALSE;
}
/*******************************************************************************
函 数 名:    USARTx_Init
功能说明:   串口1初始化函数
参    数:     pclk2:  PCLK2时钟频率(Mhz)
            bound:  波特率
返 回 值:    无
*******************************************************************************/
void USARTx_Init(u32 pclk2, UARTCfg_t *uartCfg)
{
#if     (USART == 1)
    RCC->APB2RSTR   |= UART_CLK;    //复位串口
    RCC->APB2RSTR   &= ~(UART_CLK); //停止复位
    RCC->APB2ENR    |= UART_CLK;    //使能串口时钟
#elif   (USART == 2)
    RCC->APB1RSTR   |= UART_CLK;    //复位串口2
    RCC->APB1RSTR   &= ~(UART_CLK); //停止复位
    RCC->APB1ENR    |= UART_CLK;    //使能串口2时钟
#endif
    GPIOx_Cfg(UART_PORT, UART_TX_PIN, AF_PP_50M);  //配置串口管脚
    GPIOx_Cfg(UART_PORT, UART_RX_PIN, IN_UPDOWN);  //配置串口管脚

    UART->BRR = (pclk2 * 1000000) / Bound_Rate[uartCfg->bound - 1]; // 波特率设置
    switch (uartCfg->check) //校验位
    {
    case 2:
        UART->CR1 |= (1 << 9);  //使能校验控制 / 奇校验
        break;
    case 1:
        UART->CR1 |= (1 << 10);         //使能校验控制、偶校验
        UART->CR1 |= (1 << 12);         //9个数据位 一个停止位
        break;
    default:
        break;                         //无校验
    }
    switch (uartCfg->dataBit) //有效数据位
    {
//			case 7:
//				UART->CR1 &= ~(1 << 12);     //8个数据位,stm32串口没有单独7个数据位，使用7个数据位时需要有校验位，不然会出错
//					break;
			case 8:
						if((1 == uartCfg->check) || (2 == uartCfg->check))//有校验，数据位长度加1
						{
							UART->CR1 |= (1 << 12);     //9个数据位
						}
						else	//没有校验时
						{
							UART->CR1 &= ~(1 << 12);     //8个数据位
						}
						break;
			default :
					break;
    }
    switch (uartCfg->stopBit)   //数据停止位
    {
    case 3:
        UART->CR2 |= (3 << 12); //1.5
        break;
    case 2:
        UART->CR2 |= (2 << 12); //2
        UART->CR2 &= ~(1 << 12);
        break;
    case 1:   
				UART->CR2 &= ~(3 << 12); //1  
				break;
//    case 4:
//        UART->CR2 |= (1 << 12);
//        UART->CR2 &= ~(2 << 12);//0.5
//				break;
    default:
        break;
    }

    UART->CR1 |= (1 << 13) | (1 << 3) | (1 << 2); //(UE)使能usart1 (TE)使能发送 (RE)使能接收
    UART->CR1 |= (1 << 5);                  //(RXNEIE)接收中断使能

#if     (USART == 1)
    MY_NVIC_Init(2, 2, USART1_IRQn, 2);
#elif   (USART == 2)
    MY_NVIC_Init(2, 2, USART2_IRQn, 2);
#endif
}

/*******************************************************************************
函 数 名：  USARTx_WriteBit
功能说明： 串口写入一个字节数据
参   数：    ch: 要写入的数据
返 回 值：  写入结果TRUE/FALSE
*******************************************************************************/
u8 USARTx_Write(u8 c)
{
    u32 t;

    for (t = 0; ((UART->SR & 0X40) == 0) && (t < 60000); t++) //等待数据发送完毕
    {}
			if (t < 60000/*20000*/)  //未超时//  /* 将超时时间调大一些，不然当波特率较低时发送一个字节就会超时*/
    {
        UART->DR = (u8)c;
        return TRUE;
    }
		else
    return FALSE;
}

/*******************************************************************************
函 数 名：  USARTx_Send
功能说明： 串口数据发送
参   数：    *data: 要发送的数内容
            len :   数据长度
返 回 值：  发送结果 TRUE/FALSE
*******************************************************************************/
u8 USARTx_Send(u8 *data, u16 len)
{
    while (len--)
    {
        if (!USARTx_Write(*data++))
        {
            return FALSE;
        }
    }

    return TRUE;
}

#if     (USART == 1)
/*******************************************************************************
函 数 名:    USART1_IRQHandler
功能说明:   串口1中断数据接收函数
参    数:     无
返 回 值:    无
*******************************************************************************/
void USART1_IRQHandler(void)
{
    u8 i;

    if (UART->SR & (1 << 5)) //接收到数据
    {
        i = UART->DR;
    }
}

#elif   (USART == 2)
/*******************************************************************************
函 数 名:    USART1_IRQHandler
功能说明:   串口1中断数据接收函数
参    数:     无
返 回 值:    无
*******************************************************************************/
void USART2_IRQHandler(void)
{

	Clr_UART_RecTimeOut();//

	Thread_Logout(CheckQueryRecieve);
	Thread_Login(FOREVER,0,100,&CheckQueryRecieve);

	Thread_Logout(UART_RecTimeOutCnt);
	Thread_Login(FOREVER, 0, 5, UART_RecTimeOutCnt); //检测串口数据接收是否超时
	
    UART_Recieve.dataRecieveFlag = TRUE;
	
	
    if (UART->SR & (1 << 5)) //
    {
        UART_Recieve.data[UART_Recieve.Cnt] = UART->DR;
    }
		UART_Recieve.Cnt ++;
}
#endif

/**************************Copyright BestFu 2014-05-14*************************/
