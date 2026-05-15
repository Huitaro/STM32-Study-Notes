#include "stm32f10x.h"

volatile uint32_t currentTime = 0;

void App_Delay_ms(uint32_t ms);

void App_TIM3_TimeBaseInit(void);
void App_TIM3_IT_Config(void);
void App_OnMyBoardLED_Init(void);

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	App_TIM3_TimeBaseInit();
	App_OnMyBoardLED_Init();
	
	while(1)
	{
		// 一闪一闪亮晶晶
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
		
		App_Delay_ms(50);
		
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
		
		App_Delay_ms(500);
		
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
		
		App_Delay_ms(50);
		
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
		
		App_Delay_ms(500);
		
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
		
		App_Delay_ms(800);
		
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
		
		App_Delay_ms(500);
	}
}

void App_Delay_ms(uint32_t ms)
{
	uint32_t expireTime = currentTime + ms;
	while(currentTime < expireTime);
}

void App_TIM3_TimeBaseInit(void)
{
	// #1. 打开TIM3的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	// #2. 配置时基单元的参数
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	TIM_TimeBaseInitStructure.TIM_Prescaler = 71;
	TIM_TimeBaseInitStructure.TIM_Period = 999;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	
	// 闭合自动重装载的寄存器预加载开关
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	
	// 闭合时基单元的总开关
	TIM_Cmd(TIM3, ENABLE);
	
	// #3. 设置TIM3的相关中断参数配置
	App_TIM3_IT_Config();
}

void App_TIM3_IT_Config(void)
{
	// #1. 配置Update的中断
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	// #2. 配置中断参数
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
}

void App_OnMyBoardLED_Init(void)
{
	// #1. 打开GPIOC的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	// #2. 设置GPIOC的参数
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// #3. 默认高电平
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetFlagStatus(TIM3, TIM_FLAG_Update) == SET)
	{
		TIM_ClearFlag(TIM3, TIM_FLAG_Update);
		
		currentTime++;
	}
}
