#include "stm32f10x.h"
#include "delay.h"

void App_Button_Init(void);
void App_OnMyBoard_Init(void);

int main(void)
{
	App_Button_Init();
	App_OnMyBoard_Init();
	
	uint8_t previous = Bit_SET, current = Bit_SET;
		
	while(1)
	{
		// 记录之前的状态
		previous = current;
		
		// 记录当前的状态
		current = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
		
		if(current != previous) // 按下按键的时刻
		{
			// 按键消抖
			Delay(10); 
			
			// 再次读取按键信号，防止假信号出现
			current = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
			
			if(current == Bit_SET) // 松开了按键的时刻
			{
				if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == Bit_SET)
				{
					GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);	// 点亮LED
				}
				else
				{
					GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);	// 熄灭LED
				}
			}
		}
		else{}
	}
}

//
//	@简介：对按钮进行初始化
//
void App_Button_Init(void)
{
	// PA0 - 上拉输入
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}	

//
//	@简介：对板载LED进行初始化
//
void App_OnMyBoard_Init(void)
{
	// PC13 - 开漏输出
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// 默认为高电平状态，即熄灭LED
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}
