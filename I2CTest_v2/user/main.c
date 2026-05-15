#include "stm32f10x.h"

#define I2C_OK				 		 0
#define I2C_ERR_START			-1
#define I2C_ERR_ADDR			-2
#define I2C_ERR_DATA			-3
#define I2C_ERR_TIMEOUT		-4
#define I2C_ERR_BUSY			-5
#define I2C_TIME_OUT			10000

void My_I2C_Init(void);
int My_I2C_SendBytes(I2C_TypeDef *I2Cx, uint8_t Addr, uint8_t *pData, uint16_t Size);
static int My_I2C_WaitFlag(I2C_TypeDef *I2Cx, uint32_t flag, FlagStatus status);

int main(void)
{
	My_I2C_Init();
	
	uint8_t commands[] = {
		0x00, // 命令流
		0x8d, 0x14, // 使能电荷泵
		0xaf, // 打开屏幕开关
		0xa5, // 让屏幕全亮
	};
	
	My_I2C_SendBytes(I2C1, 0x78, commands, 5);
	
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
	int ret;
	int timeOut = I2C_TIME_OUT;
	
	// #1. 等待总线空闲
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) == SET)
	{
		if(--timeOut == 0)
		{
			return I2C_ERR_BUSY;	// 超时处理，防止程序死机
		}
	}
	
	// #2. 发送起始位
	I2C_GenerateSTART(I2Cx, ENABLE);
	// 等待起始位发送成功
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) == RESET)
	{
		if(--timeOut == 0)
		{
			return I2C_ERR_START; // 超时处理，防止程序死机
		}
	}
	
	// #3. 寻址阶段
	I2C_ClearFlag(I2Cx, I2C_FLAG_AF); // 清除AF标志位
	I2C_Send7bitAddress(I2Cx, Addr, I2C_Direction_Transmitter);
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
			return I2C_ERR_ADDR; //寻址失败
		}
		
		if(--timeOut == 0)
		{
			return I2C_ERR_TIMEOUT; // 超时处理，防止程序死机
		}
	}
	
	// 读SR1和SR2寄存器，清除ADDR标志
	(void)I2C_ReadRegister(I2Cx, I2C_Register_SR1);
	(void)I2C_ReadRegister(I2Cx, I2C_Register_SR2);
	
	// #4. 发送数据
	for(uint16_t i=0; i<Size; i++)
	{
		ret = My_I2C_WaitFlag(I2Cx, I2C_FLAG_TXE, SET);
		if(ret != I2C_OK)
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return ret;
		}
		I2C_SendData(I2Cx, pData[i]);
	}
	
	// 检测最后一个数据是否发送成功
	ret = My_I2C_WaitFlag(I2Cx, I2C_FLAG_BTF, SET);
	if(ret != I2C_OK)
	{
		I2C_GenerateSTOP(I2Cx, ENABLE);
		return ret;
	}
	
	// #5. 发送停止位
	// 发送停止位
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// 数据发送成功
	return I2C_OK;
}

static int My_I2C_WaitFlag(I2C_TypeDef *I2Cx, uint32_t flag, FlagStatus status)
{
	int timeOut = I2C_TIME_OUT;
	
	while(I2C_GetFlagStatus(I2Cx, flag) != status)
	{
		if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET)
		{
			return I2C_ERR_DATA; // 发送数据被拒绝
		}
	}
	
	if(--timeOut == 0)
	{
		return I2C_ERR_TIMEOUT;	// 超时处理，防止程序死机
	}
	
	return I2C_OK;
}

