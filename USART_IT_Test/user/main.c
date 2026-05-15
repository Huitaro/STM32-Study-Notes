#include "stm32f10x.h"
//#include "usart.h"
#include "delay.h"

uint32_t blinkInterval = 1000;

void App_OnMyBoardLED_Init(void);
void App_USART_Init(void);
void App_USART_IT_Init(void);
void USART1_IRQHandler(void);

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	App_OnMyBoardLED_Init();
	App_USART_Init();
	
//	My_USART_SendString(USART1, "Hello World!! \r\n");
	
	while(1)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET); // 亮
		Delay(blinkInterval);
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET); // 灭
		Delay(blinkInterval);
	}
}

//
//	@简介：USART的中断触发函数
//
void USART1_IRQHandler(void)
{
	// 等待数据寄存器为空时，才可接收数据
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
	{
		uint16_t DataRcvd = USART_ReceiveData(USART1);
		
		if(DataRcvd == '0') 
		{
			blinkInterval = 1500;
		}
		else if(DataRcvd == '1') 
		{
			blinkInterval = 500;
		}
		else if(DataRcvd == '2') 
		{
			blinkInterval = 100;
		}
	}
	
}

//
//	@简介：对板载LED->PC13进行初始化
//
void App_OnMyBoardLED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// 默认为关闭状态
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
	
}

//
//	@简介：对USART1进行初始化
//
void App_USART_Init(void)
{
	// #1. 对PA9、PA10进行初始化
	// PA9->Tx		PA10->Rx
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// #2. 初始化USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART1, &USART_InitStructure);
	
	// #3. 闭合USART1的总开关
	USART_Cmd(USART1, ENABLE);
	
	// #4. 配置USART的中断
	App_USART_IT_Init();
	
}

//
//	@简介：对USART的中断进行相关配置
//
void App_USART_IT_Init(void)
{
	// 使能USART中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	// 配置NVIC参数
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//0~3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				//0~3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
	
}

