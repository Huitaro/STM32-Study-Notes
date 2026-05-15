#include "stm32f10x.h"

int main(void)
{
	// #1. 开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	// #2. 初始化IO引脚，PA0，通用推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//	GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET); // 写1点亮
	
	// #3. 初始化IO引脚，PA1，上拉输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	while(1)
	{
		if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_RESET )
		{
			GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
		}
		else
		{
			GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
		}
	}
}
