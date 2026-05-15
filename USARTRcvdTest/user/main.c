#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"

void My_USART1_Init(void);
void My_OnBoard_LED_Init(void);

int main(void)
{
	My_USART1_Init();
	My_OnBoard_LED_Init();
	
	while(1)
	{
		// #1. 等待接收数据寄存器为非空
		while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
		
		// #2. 获取接收数据寄存器的数据
		uint8_t byteRcvd = USART_ReceiveData(USART1);
		
		// #3. 根据获得的数据点亮灭板载的LED
		if(byteRcvd == '1')
		{
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);  // 点亮
		}
		else if(byteRcvd == '0')
		{
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET); 	// 熄灭
		}
	}
}

//
//	@简介：对串口1进行初始化
//	PB6 -Tx		PB7 -Rx
//	配置参数：115200  8数据位  1停止位  无校验位  
//
void My_USART1_Init(void)
{
	//	// # 开启GPIOA的时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	
//	GPIO_InitTypeDef GPIO_Initstructure;
//	// # PA9 Tx 复用推挽输出
//	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_9;
//	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_10MHz;
//	GPIO_Init(GPIOA, &GPIO_Initstructure);
//	// # PA10 Rx 浮空输入或上拉输入
//	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_10;
//	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_Init(GPIOA, &GPIO_Initstructure);
	
	// # 1.初始化PB6、PB7
	// # 打开AFIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	// # USART1_Remap = 1
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	
	// # 打开GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_Initstructure;
	// # PB6 Tx 复用推挽输出
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_Initstructure);
	// # PB7 Rx 浮空输入或上拉输入
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_Initstructure);
	
	// # 2.初始化USART1
	// # 开启USART1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	// # 初始化USART1
	USART_InitTypeDef USART1_InitStructure;
	
	USART1_InitStructure.USART_BaudRate = 115200;
	USART1_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART1_InitStructure.USART_Parity = USART_Parity_No;
	USART1_InitStructure.USART_StopBits = USART_StopBits_1;
	
	USART_Init(USART1, &USART1_InitStructure);
	
	// # 打开USART的总开关
	USART_Cmd(USART1, ENABLE);
}

//
//	@简介：对板载LED进行初始化
//	板载LED -PC13
//
void My_OnBoard_LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

