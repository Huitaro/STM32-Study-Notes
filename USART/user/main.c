#include "stm32f10x.h"

void My_USART_SendBytes(USART_TypeDef *USARTx, uint16_t *pData, uint16_t Size);

int main(void)
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
	
	// # 向串口发送数据
	uint16_t bytesToSend[] = {1, 2, 3, 4, 5};
	
	My_USART_SendBytes(USART1, bytesToSend, 5);
	
	while(1)
	{
	}
}

void My_USART_SendBytes(USART_TypeDef *USARTx, uint16_t *pData, uint16_t Size)
{
	for(uint32_t i = 0; i < Size; i++)
	{
		// # 等待发送数据寄存器为空
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET); // RESET=0:空		SET=1:非空
		
		// # 通过串口发送数据
		USART_SendData(USARTx, pData[i]);
	}
	
	// # 等待发送数据寄存器和移位寄存器为空，即数据发送完成
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}
