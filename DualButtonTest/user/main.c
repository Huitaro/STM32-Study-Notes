#include "stm32f10x.h"

void App_OnMyBoardLED_Init(void);
void App_Button_Init(void);
void App_Button_IT_Init(void);

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	App_OnMyBoardLED_Init();
	App_Button_Init();
	
	while(1)
	{
	}
}

void App_OnMyBoardLED_Init(void)
{
	// PC13
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// 默认熄灭LED
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}

void App_Button_Init(void)
{
	// #1. 初始化PA5、PA6
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	// PA5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// PA6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// #2. 配置EXTI的线
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);
	
	// #3. 初始化EXTI
	EXTI_InitTypeDef EXTI_InitStructure = {0};
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	// #4. 配置中断
	App_Button_IT_Init();
}

void App_Button_IT_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure = {0};
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
}

void EXTI9_5_IRQHandler(void)
{
	// 按下按键5
	if(EXTI_GetFlagStatus(EXTI_Line5) == SET)
	{
		EXTI_ClearFlag(EXTI_Line5);	// 清除中断标志位
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);	// 点亮LED
	}
	
	// 按下按键6
	if(EXTI_GetFlagStatus(EXTI_Line6) == SET)
	{
		EXTI_ClearFlag(EXTI_Line6);	// 清除中断标志位
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);	// 熄灭LED
	}
}
