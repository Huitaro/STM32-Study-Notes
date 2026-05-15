#include "stm32f10x.h"
#include "button.h"

Button_TypeDef Button;

void App_SPI1_Init(void);
void App_SPI_MasterTransmitReceive(SPI_TypeDef * SPIx, const uint8_t *pDataTx, uint8_t *pDataRx, uint16_t Size);

void App_W25Q64_SaveByte(uint8_t Byte);
uint8_t App_W25Q64_LoadByte(void);

void App_OnMyBoardLED_Init(void);
void App_MyButton_Init(void);
void button_clicked_cb(uint8_t clicks);

int main(void)
{
	App_SPI1_Init();
	App_OnMyBoardLED_Init();
	App_MyButton_Init();
	
	uint8_t byte = App_W25Q64_LoadByte();
	if(byte == 0)
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
	}
	else
	{
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
	}
	
	while(1)
	{
		My_Button_Proc(&Button);
	}
}

//
//	@简介：编写按钮敲击的回调函数
//
void button_clicked_cb(uint8_t clicks)
{
	if(clicks == 1)
	{
		if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET)	// 当前LED处于点亮状态
		{
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);	//	熄灭LED	
			App_W25Q64_SaveByte(0x00);
		}
		else	// 当前LED处于熄灭状态
		{
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);	//	点亮LED	
			App_W25Q64_SaveByte(0x01);
		}
	}
}

//
//	@简介：初始化按钮
//
void App_MyButton_Init(void)
{
	// PA0
	Button_InitTypeDef Button_InitStructure = {0};
	
	Button_InitStructure.GPIOx = GPIOA;
	Button_InitStructure.GPIO_Pin = GPIO_Pin_0;
	Button_InitStructure.button_clicked_cb = button_clicked_cb;
	
	My_Button_Init(&Button, &Button_InitStructure);
}

//
//	@简介：初始化板载LED
//
void App_OnMyBoardLED_Init(void)
{
	// PC13
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}

//
//	@简介：对SPI1进行初始化
//
void App_SPI1_Init(void)
{
	// #1. 对引脚IO进行初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
		
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	// PB3->SPI1_CLK		AF_PP	 2MHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// PB4->SPI1_MISO		IPU
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// PB5->SPI1_MOSI		AF_PP	 2MHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// PA15  Out_PP	 2MHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);	// 片选信号设为高电压，建立与从机的通信
	
	// #2. 对SPI1进行初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	SPI_InitTypeDef SPI_InitStructure = {0};
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
}

//
//	@简介：将STM32芯片作为主机，实现数据收发的功能
//
void App_SPI_MasterTransmitReceive(SPI_TypeDef * SPIx, const uint8_t *pDataTx, uint8_t *pDataRx, uint16_t Size)
{
	// #1. 闭合总开关
	SPI_Cmd(SPIx, ENABLE);
	
	// #2. 发送第一个字节数据
	SPI_I2S_SendData(SPIx, pDataTx[0]);
	
	for(uint16_t i=0; i<Size-1; i++)
	{
		// #3. 向TDR写数据
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPIx, pDataTx[i+1]);
		
		// #4. 向RDR读数据
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);
		pDataRx[i] = SPI_I2S_ReceiveData(SPIx);
	}
	
	// #5. 读取最后一个数据
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);
	pDataRx[Size-1] = SPI_I2S_ReceiveData(SPIx);
	
	// #6. 断开总开关
	SPI_Cmd(SPIx, DISABLE);
}

//
//	@简介：向W25Q64的目标地址存储一个字节数据
//
void App_W25Q64_SaveByte(uint8_t Byte)
{
	uint8_t Buffer[10];
	// #1. 写使能
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);	// 选中
	
	Buffer[0] = 0x06;
	App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 1);	
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);	// 取消选中
	
	// #2. 扇区擦除
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);	// 选中
	
	Buffer[0] = 0x20;
	Buffer[1] = 0x00;
	Buffer[2] = 0x00;
	Buffer[3] = 0x00;
	
	App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 4);
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);	// 取消选中
	
	// #3. 等待空闲
	while(1)
	{
		GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);	// 选中
	
		Buffer[0] = 0x05;
		App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 1);
	
		Buffer[0] = 0xff;
		App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 1);
	
		GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);	// 取消选中
		if((Buffer[0] & 0x01) == 0) break;
	}
	
	// #4. 写使能
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);	// 选中
	
	Buffer[0] = 0x06;
	App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 1);	
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);	// 取消选中
	
	// #5. 页编程
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);	// 选中
	
	Buffer[0] = 0x02;
	Buffer[1] = 0x00;
	Buffer[2] = 0x00;
	Buffer[3] = 0x00;
	Buffer[4] = Byte;
	App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 5);
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);	// 取消选中
	
	// #6. 等待空闲
	while(1)
	{
		GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);	// 选中
	
		Buffer[0] = 0x05;
		App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 1);
	
		Buffer[0] = 0xff;
		App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 1);
	
		GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);	// 取消选中
		if((Buffer[0] & 0x01) == 0) break;
	}
}

//
//	@简介：向W25Q64的目标地址读取一个字节数据
//
uint8_t App_W25Q64_LoadByte(void)
{
	uint8_t Buffer[10];
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET); //选中
	
	Buffer[0] = 0x03;
	Buffer[1] = 0x00;
	Buffer[2] = 0x00;
	Buffer[3] = 0x00;
	App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 4);
	
	Buffer[0] = 0xff;
	App_SPI_MasterTransmitReceive(SPI1, Buffer, Buffer, 1);
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);	// 取消选中
	
	return Buffer[0];
}
