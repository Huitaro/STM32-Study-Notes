#include "stm32f10x.h"
//#include "i2c.h"
#include "si2c.h"

SI2C_TypeDef si2c;

//void My_I2C_init(void);
void My_OnBoard_LED_Init(void);

int main(void)
{
	si2c.SCL_GPIOx = GPIOB;
	si2c.SCL_GPIO_Pin = GPIO_Pin_6;
	si2c.SDA_GPIOx = GPIOB;
	si2c.SDA_GPIO_Pin = GPIO_Pin_7;
	
	My_SI2C_Init(&si2c);
	
	//My_I2C_init();
	My_OnBoard_LED_Init();
	
	uint8_t commands[] = {
    0x00,  // 命令流
    0x8d, 0x14,  // 使能电荷泵
    0xaf,  // 打开屏幕开关
    0xa5   // 让屏幕全亮
   };
	
	//My_I2C_SendBytes(I2C1, 0x78, commands, 5);
	My_SI2C_SendBytes(&si2c, 0x78, commands, 5);
	 
	uint8_t rcvd;
	 
	//My_I2C_ReceiveBytes(I2C1, 0x78, &rcvd, 1);
	My_SI2C_ReceiveBytes(&si2c, 0x78, &rcvd, 5);
	 
	if((rcvd & (0x01 << 6)) == 0)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
	}
	else
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
	}
	 
	while(1)
	{
	}
}

////
////	@简介：初始化I2C1
////
//void My_I2C_init()
//{
//	// #1. 对PB6、PB7进行初始化
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	
//	GPIO_InitTypeDef GPIO_InitStructure;
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;		// 对应是I2C，所以使用复用功能输出
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//	
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	
//	// #2. 对I2C1进行初始化
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
//	
//	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);	// 使能复位信号
//	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);	// 释放复位信号
//	
//	I2C_InitTypeDef I2C_InitStructure;
//	
//	I2C_InitStructure.I2C_ClockSpeed = 400000;
//	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
//	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
//	
//	I2C_Init(I2C1, &I2C_InitStructure);
//	
//	// #3. 打开I2C1的总开关
//	I2C_Cmd(I2C1, ENABLE);
//}

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
	
	// 默认设置为高电平状态
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}
