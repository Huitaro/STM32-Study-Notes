#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"

void My_USART_SendBytes(USART_TypeDef *USARTx, uint16_t *pData, uint16_t Size);
void My_USART1_Init(void);
void PrintfCurrentTime(void);

int main(void)
{
	My_USART1_Init();
	
	//printf("Hello World\r\n");
	
	while(1)
	{
		PrintfCurrentTime();
		Delay(100);
	}
}

//
//	@简介：向串口发送数据
//	@参数 USARTx：串口的名称
//	@参数 pData：要发送的数据
//	@参数 Size：发送的大小，单位为字节
//
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

// # 重写fputc函数
int fputc(int ch, FILE *f)
{
	// # 1.等待发送寄存器为空
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	
	// # 2.将数据写入发送数据寄存器
	USART_SendData(USART1, (uint8_t)ch);
	
	return ch;
}

//
//	@简介：串口打印当前的时间
//
void PrintfCurrentTime(void)
{
	// #1. 获取当前时间
	uint32_t current_time = GetTick();
	
	// #2. 获取毫秒数
	uint32_t milliscend = current_time % 1000;
	current_time /= 1000;
	
	// #3. 获取秒数
	uint32_t second = current_time % 60;
	current_time /= 60;
	
	// #4. 获取分钟数
	uint32_t minute = current_time % 60;
	current_time /= 60;
	
	// #5. 获取小时数
	uint32_t hour = current_time;
	
	// #6. 串口输出
	printf("%02u:%02u:%02u.%03u", hour, minute, second, milliscend);
}
