#include "stm32f10x.h"

void My_Soft_I2C_Init(void);

void scl_write(uint8_t level);
void sda_write(uint8_t level);
uint8_t sda_read(void);

void delay_us(uint32_t us);

void SendStart(void);
void SendStop(void);

uint8_t SendByte(uint8_t Byte);
uint8_t ReceiveByte(uint8_t Ack);

int My_Soft_I2C_SendBytes(uint16_t Addr, uint8_t *pData, uint32_t Size);
int My_Soft_I2C_ReceiveBytes(uint16_t Addr, uint8_t *pBuffer, uint32_t Size);

int main(void)
{
	My_Soft_I2C_Init();
	
	uint8_t commands[] = {
    0x00,  // 命令流
    0x8d, 0x14,  // 使能电荷泵
    0xaf,  // 打开屏幕开关
    0xa5   // 让屏幕全亮
  };
	
	My_Soft_I2C_SendBytes(0x78, commands, 5);
	
	while(1)
	{
	}
}

//
//	简介：软件初始化I2C
//
void My_Soft_I2C_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// SCL->PA0		SDA->PA1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 初始化后将SCL、SDA拉高
	GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
	GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
}

//
//	简介：向SCL写操作
//
void scl_write(uint8_t level)
{
	if(level == 1)
	{
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
	}
	else
	{
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
	}
}

//
//	简介：向SDA写操作
//
void sda_write(uint8_t level)
{
	if(level == 1)
	{
		GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
	}
	else
	{
		GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
	}
}

//
//	简介：向SDA读操作
//
uint8_t sda_read(void)
{
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_SET)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//
//	简介：简单的微秒级延迟操作
//
void delay_us(uint32_t us)
{
	uint32_t n = us * 8;
	for(uint32_t i = 0; i < n; i++);
}

//
//	简介：发送起始位
//
void SendStart(void)
{
	// SCL->高电平（默认）		SDA->下降沿操作
	sda_write(0);
	delay_us(1);
}

//
//	简介：发送停止位
//
void SendStop(void)
{
	// SCL->高电平		SDA->上升沿操作
	scl_write(0);
	sda_write(0);
	delay_us(1);
	
	scl_write(1);
	delay_us(1);
	
	sda_write(1);
	delay_us(1);
}

//
//	简介：发送一位字节数据
//
uint8_t SendByte(uint8_t Byte)
{
	for(int8_t i = 7; i>=0; i--)
	{
		// SCL拉低，准备数据
		scl_write(0);
		if((Byte & (0x01 << i)) != 0)
		{
			sda_write(1);
		}
		else
		{
			sda_write(0);
		}
		// SCL拉高，发送数据
		scl_write(1);
		delay_us(1);
	}
	
	// 返回ACK或NAK
	// 拉高SDA，表示主机发送完数据
	scl_write(0);
	sda_write(1);
	delay_us(1);
	
	// 等待从机SDA拉低或不回应
	scl_write(1);
	delay_us(1);
	return sda_read();
}

//
//	@简介：接收一位字节数据
//
uint8_t ReceiveByte(uint8_t Ack)
{
	uint8_t Byte = 0;
	for(int8_t i = 7; i >= 0; i--)
	{
		// SDA拉高，等待从机发送数据
		scl_write(0);
		sda_write(1);
		delay_us(1);
		
		scl_write(1);
		delay_us(1);
		
		if(sda_read() != 0)
		{
			Byte |= (0x01 << i);
		}
	}
	
	// 返回ACK或NAK
	scl_write(0);
	sda_write(!Ack);	// Ack=0，即主机不继续接受，拉高SDA；Ack=1，主机继续接收，拉低SDA
	delay_us(1);
	
	scl_write(1);
	delay_us(1);
	
	return Byte;
}

//
//	@简介：实现软件I2C发送数据
//
int My_Soft_I2C_SendBytes(uint16_t Addr, uint8_t *pData, uint32_t Size)
{
	// #1. 发送起始位
	SendStart();
	
	// #2. 寻址阶段
	if(SendByte(Addr & 0xfe) != 0)
	{
		// 收到NAK，即SDA未拉低
		SendStop();
		return -1;	// 寻址失败
	}		
	
	// #3. 发送数据
	for(uint32_t i = 0; i < Size; i++)
	{
		if(SendByte(pData[i]) != 0)
		{
			// 收到NAK，即SDA未拉低
			SendStop();
			return -2;	// 发送数据失败
		}
	}
	
	// #4. 发送停止位
	SendStop();
	
	return 0; // 发送成功
}

//
//	@简介：实现软件I2C接收数据
//
int My_Soft_I2C_ReceiveBytes(uint16_t Addr, uint8_t *pBuffer, uint32_t Size)
{
	// #1. 发送起始位
	SendStart();
	
	// #2. 寻址阶段
	if(SendByte(Addr | 0x01) != 0)
	{
		// 收到NAK，即SDA未拉低
		SendStop();
		return -1;	// 寻址失败
	}
	
	// #3. 接收数据
	for(uint32_t i = 0; i < Size - 1; i--)
	{
		pBuffer[i] = ReceiveByte(1);
	}
	
	pBuffer[Size - 1] = ReceiveByte(0); // 接收完最后一个数据，返回NAK
	
	// #4. 发送停止位
	SendStop();
	
	return 0; // 接收成功
}
