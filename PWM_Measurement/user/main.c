#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include <math.h>

void App_USART1_Init(void);
void App_TIM3_Init(void);
void App_TIM1_Init(void);

int main(void)
{
	App_USART1_Init();
	App_TIM3_Init();
	App_TIM1_Init();
	
	// 设置PWM的占空比为20%
	TIM_SetCompare1(TIM3, 200);
	
//	My_USART_SendString(USART1, "你好世界！！\r\n");
	
	while(1)
	{
		// #1. 清除Trigger标志位
		TIM_ClearFlag(TIM1, TIM_FLAG_Trigger);
		
		// #2. 等待Trigger标志位置1
		while(TIM_GetFlagStatus(TIM1, TIM_FLAG_Trigger) == RESET);
		
		// #3. 计算PWM周期、占空比
		uint16_t ccr1 = TIM_GetCapture1(TIM1);
		uint16_t ccr2 = TIM_GetCapture2(TIM1);
		
		float period = ccr1 * 1.0e-3f;
		float duty = ((float)ccr2) / ccr1 * 100.0f;
		
		My_USART_Printf(USART1, "周期=%.3f ms，占空比=%.2f %%", period, duty);
		Delay(100);
		
//		float t = GetTick() * 1.0e-3f;	// 获取当前时间ms，并转换为s
//		float duty = 0.5 * (sin(2 * 3.1415926 * t) + 1);
//		
//		uint16_t ccr1 = duty * 999;
//		TIM_SetCompare1(TIM3, ccr1);
	}
}

//
//	@简介：初始化串口1
//
void App_USART1_Init(void)
{
	// #1. 初始化GPIO
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	// PA9->TX	AF_PP
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// PA10->RX	 IPU
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// #2. 初始化USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitTypeDef USART_InitStructure = {0};
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_WordLength = 	USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	
	USART_Init(USART1, &USART_InitStructure);
	
	// #3. 闭合USART1的开关
	USART_Cmd(USART1, ENABLE);
}

//
//	@简介：初始化定时器3用于产生PWM
//
void App_TIM3_Init(void)
{
	// #1. 初始化时基单元
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
	
	TIM_TimeBaseInitStructure.TIM_Prescaler = 71;
	TIM_TimeBaseInitStructure.TIM_Period = 999;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	
	TIM_ARRPreloadConfig(TIM3, ENABLE);		// 启动ARR寄存器的预加载
	
	TIM_Cmd(TIM3, ENABLE);
	
	// #2. 初始化输出比较->通道1
	TIM_OCInitTypeDef TIM_OCInitStructure = {0};
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	
	// 闭合MOE
	TIM_CtrlPWMOutputs(TIM3, ENABLE);
	// 启动捕获/比较寄存器的预装载
	TIM_CCPreloadControl(TIM3, ENABLE);
	
	// #3. 初始化GPIO引脚
	// PA6->AF_PP
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

//
//	@简介：初始化定时器1用于捕获定时器3产生的PWM，最后测量出PWM参数：周期、占空比
//
void App_TIM1_Init(void)
{
	// #1. 初始化时基单元
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
	
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 65535;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 71;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	
	TIM_ARRPreloadConfig(TIM1, ENABLE);	// 启动ARR寄存器的预加载
	
	TIM_Cmd(TIM1, ENABLE);
	
	// #2. 初始化输入捕获
	TIM_ICInitTypeDef TIM_ICInitStructure = {0};
	
	// 通道1	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	
	TIM_ICInit(TIM1, &TIM_ICInitStructure);
	
	// 通道2
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	
	TIM_ICInit(TIM1, &TIM_ICInitStructure);
	
	// #3. 初始化从模式控制器
	TIM_SelectInputTrigger(TIM1, TIM_TS_TI1FP1);
	TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Reset);
	
	// #4. 初始化GPIO
	// PA8->IPD
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
