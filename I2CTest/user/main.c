#include "stm32f10x.h"

void My_I2C_Init(void);
int My_I2C_SendBytes(I2C_TypeDef *I2Cx, uint8_t Addr, uint8_t *pData, uint16_t Size);
int My_I2C_ReceiveBytes(I2C_TypeDef *I2Cx, uint8_t Addr, uint8_t *pBuffer, uint16_t Size);

void My_OnBoard_LED_Init(void);

int main(void)
{
	My_I2C_Init();
	My_OnBoard_LED_Init();
	
	uint8_t commands[] = {
		0x00, // 命令流
		0x8d, 0x14, // 使能电荷泵
		0xaf, // 打开屏幕开关
		0xa5, // 让屏幕全亮
	};
	
	My_I2C_SendBytes(I2C1, 0x78, commands, 5);
	
	uint8_t Rcvd;
	My_I2C_ReceiveBytes(I2C1, 0x78, &Rcvd, 1);
	if( (Rcvd & (0x01 << 6)) == 0)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET); // 点亮
	}
	else
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);	// 熄灭
	}
	
	while(1)
	{
	}
}

void My_I2C_Init(void)
{
	// #1. 对I/O引脚初始化
	// 打开AFIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);		// 对I2C1进行重映射
	
	// 打开GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 初始化GPIO
	// PB8-->SCL	PB9-->SDA
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// #2. 对I2C1进行初始化
	// 打开I2C1的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	// 对I2C1进行复位（重新启动）
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);	// 施加复位信号
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);	// 释放复位信号
	
	// 配置I2C1的参数
	I2C_InitTypeDef I2C1_InitStructure;
	I2C1_InitStructure.I2C_ClockSpeed = 400000;
	I2C1_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C1_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_Init(I2C1, &I2C1_InitStructure);
	
	// 使能I2C1，即打开I2C1的总开关
	I2C_Cmd(I2C1, ENABLE);
}

int My_I2C_SendBytes(I2C_TypeDef *I2Cx, uint8_t Addr, uint8_t *pData, uint16_t Size)
{
	// #1. 等待总线空闲
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) == SET);
	
	// #2. 发送起始位
	// 发送起始位
	I2C_GenerateSTART(I2Cx, ENABLE);
	// 等待起始位发送成功
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) == RESET);
	
	// #3. 寻址阶段
	// 清除AF标志位
	I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
	I2C_SendData(I2Cx, Addr & 0xfe);	// 写数据，即R/W# = 0
	while(1)
	{
		// 地址发送成功
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR) == SET)
		{
			break;
		}
		
		// 地址还未发送成功，收到了AF应答失败的信号，即寻址失败
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
		{
			// 发送停止位
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return -1; //寻址失败
		}
	}
	
	// 向SR1和SR2读寄存器
	I2C_ReadRegister(I2Cx, I2C_Register_SR1);
	I2C_ReadRegister(I2Cx, I2C_Register_SR2);
	
	// #4. 发送数据
	for(uint16_t i=0; i<Size; i++)
	{
		while(1)
		{
			// 收到应答失败的信号，即代表发送的数据被拒绝
			if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
			{
				// 发送停止位
				I2C_GenerateSTOP(I2Cx, ENABLE);
				return -2; // 发送数据被拒绝
			}
			
			// 应答成功，此时检查发送数据寄存器是否为空
			if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) == SET)
			{
				break; // 数据寄存器为空，此时可跳出循环开始发送数据
			}
		}
		I2C_SendData(I2Cx, pData[i]);
	}
	
	// 发送最后一个数据
	while(1)
	{
		// 收到的应答失败的信号，即代表发送的数据被拒绝
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
		{
			// 发送停止位
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return -2; // 发送数据被拒绝
		}
		
		// 收到应答信号，接着检测数据是否发送完毕
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF) == SET)
		{
			break; // 数据全部发送完成，退出此循环
		}
	}
	
	// #5. 发送停止位
	// 发送停止位
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// 数据发送成功
	return 0;
}

int My_I2C_ReceiveBytes(I2C_TypeDef *I2Cx, uint8_t Addr, uint8_t *pBuffer, uint16_t Size)
{
	// #1. 发送起始位
	I2C_GenerateSTART(I2Cx, ENABLE);
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) == RESET); // 等待发送成功
	
	// #2. 寻址阶段
	I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
	I2C_SendData(I2Cx, Addr | 0x01);
	
	while(1)
	{
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR) == RESET)
		{
			break;
		}
		
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return -1; // 寻址失败
		}
	}
	
	// #3. 读取数据
	if(Size == 1)			// 只读取一个字节数据
	{
		// 清除ADDR，即读寄存器SR1、SR2
		I2C_ReadRegister(I2Cx, I2C_Register_SR1);
		I2C_ReadRegister(I2Cx, I2C_Register_SR2);
		
		// Ack=0，并发送停止位
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
		I2C_GenerateSTOP(I2Cx, ENABLE);
		
		pBuffer[0] = I2C_ReceiveData(I2Cx);
	}
	else if(Size == 2)		// 读取两个字节数据
	{
		// 清除ADDR，即读寄存器SR1、SR2
		I2C_ReadRegister(I2Cx, I2C_Register_SR1);
		I2C_ReadRegister(I2Cx, I2C_Register_SR2);
		
		// Ack=1，继续接收数据
		I2C_AcknowledgeConfig(I2Cx, ENABLE);
		
		// 读取第一个数据
		pBuffer[0] = I2C_ReceiveData(I2Cx);
		
		// Ack=0，并发送停止位
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
		I2C_GenerateSTOP(I2Cx, ENABLE);
		
		pBuffer[1] = I2C_ReceiveData(I2Cx);
	}
	else		// 读取两个字节以上数据
	{
		// 清除ADDR，即读寄存器SR1、SR2
		I2C_ReadRegister(I2Cx, I2C_Register_SR1);
		I2C_ReadRegister(I2Cx, I2C_Register_SR2);
		
		// 循环读取Size-1个数据
		for(uint16_t i = 0; i < Size-1; i++)
		{
			// Ack=1
			I2C_AcknowledgeConfig(I2Cx, ENABLE);
			pBuffer[i] = I2C_ReceiveData(I2Cx);
		}
		
		// 读取最后一个数据前，预先Ack=0，并发送停止位
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
		I2C_GenerateSTOP(I2Cx, ENABLE);
		
		pBuffer[Size-1] = I2C_ReceiveData(I2Cx);
	}
	
	// #4. 读取数据成功
	return 0; 
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
	
	// 默认设置为高电平状态
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}
