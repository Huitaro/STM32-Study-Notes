#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"

void App_USART1_Init(void);
void App_HC_SR04_Init(void);

int main(void)
{
	App_USART1_Init();
	App_HC_SR04_Init();
	
//	My_USART_SendString(USART1, "Hello World!! \r\n");
	
	while(1)
	{
		// #1. 清零CNT、CCR1、CCR2
		TIM_SetCounter(TIM1, 0);
		
		TIM_ClearFlag(TIM1, TIM_FLAG_CC1);
		TIM_ClearFlag(TIM1, TIM_FLAG_CC2);
		
		// #2. 闭合时基单元总开关，开启定时器
		TIM_Cmd(TIM1, ENABLE);
		
		// #3. 向Trig发送脉冲
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
		DelayUs(10);
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
		
		// #4. 等待捕获完成
		while(TIM_GetFlagStatus(TIM1, TIM_FLAG_CC1) == RESET);
		while(TIM_GetFlagStatus(TIM1, TIM_FLAG_CC2) == RESET);
		
		// #5. 打开时基单元总开关，关闭定时器
		TIM_Cmd(TIM1, DISABLE);
		
		// #6. 计算距离并打印串口输出
		uint16_t ccr1 = TIM_GetCapture1(TIM1);
		uint16_t ccr2 = TIM_GetCapture2(TIM1);
		
//		float distance = (ccr2 - ccr1) * 1.0e-6f * 340.0f / 2.0f;	// 单位m
//		float distance = (ccr2 - ccr1) * 0.017f;	// 单位cm

		// 简化成工程常数，减少计算量
		
		float distance = (ccr2 - ccr1) / 58.0f;	// 单位cm
		
		My_USART_Printf(USART1, "distance = %.4f cm \r\n", distance);	// 单位cm
		Delay(100);
	}
}

void App_USART1_Init(void)
{
	// #1. 初始化GPIO引脚
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	// PA9->Tx	AF_PP
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// PA10->Rx	 IPU
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// #2. 初始化USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	
	USART_Init(USART1, &USART_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}

void App_HC_SR04_Init(void)
{
	// #1. 初始化GPIO引脚
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	// Trig->PA0	Out_PP
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// Eco->PA8	 IPD
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// #2. 初始化时基单元
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	TIM_TimeBaseInitStructure.TIM_Period = 65535;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 71;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	
	// #3. 初始化捕获输入
	TIM_ICInitTypeDef TIM_ICInitStructure;
	
	// 通道1->捕获上升沿
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0F;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	
	TIM_ICInit(TIM1, &TIM_ICInitStructure);
	
	// 通道2->捕获下降沿
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICFilter = 0x0F;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	
	TIM_ICInit(TIM1, &TIM_ICInitStructure);
}
