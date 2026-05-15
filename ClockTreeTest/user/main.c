#include "stm32f10x.h"

void App_OnMyBoardLED_Init(void);
void App_SystemClock_Init(void);

int main(void)
{
	App_SystemClock_Init();
	App_OnMyBoardLED_Init();

	while(1)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
		
		for(uint32_t i=0; i<400000; i++);
		
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
		
		for(uint32_t i=0; i<400000; i++);
	}
}

void App_SystemClock_Init(void)
{
	FLASH_PrefetchBufferCmd(ENABLE);
	FLASH_SetLatency(FLASH_Latency_2);
	
	// #1. 开启HSE
	RCC_HSEConfig(RCC_HSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
	
	// #2. 配置并开启锁相环
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
	
	// #3. 配置AHB、APB1、APB2的倍频系数
	RCC_HCLKConfig(RCC_HCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	
	// #4. 选择系统时钟源
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource() != 0x08);
}

void App_OnMyBoardLED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}
