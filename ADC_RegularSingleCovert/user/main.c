#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"

void App_ADC1_Init(void);
void App_OnMyBoardLED_Init(void);
void My_USART1_Init(void);

int main(void)
{
	App_ADC1_Init();
	App_OnMyBoardLED_Init();
	My_USART1_Init();
	
//	My_USART_SendString(USART1, "Hello World");
	
	while(1)
	{
		// #1. 清除EOC标志位
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
		
		// #2. 软件启动
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		
		// #3. 等待EOC标志位置1
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		
		// #4. 获取dr值并转换
		uint16_t dr = ADC_GetConversionValue(ADC1);
		float voltage = dr * (3.3f / 4095.0f);
		
		// #5. 判断电压值作出相关的LED亮灭操作
		if(voltage > 1.5)
		{
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);	// 弱光，熄灭LED
		}
		else
		{
			GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);	// 强光，点亮LED
		}
		
		My_USART_Printf(USART1, "Voltage = %.3fv\r\n", voltage);
		Delay(200);
		
	}
}

void App_ADC1_Init(void)
{
	// #1. 初始化GPIO引脚
	// PA0->模拟输入
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// #2. 配置ADC1的时钟频率
	// 12MHz
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 六分频 -> 72MHz / 6 = 12MHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); // 打开ADC1的时钟
	
	// #3. 初始化ADC1的参数
	ADC_InitTypeDef ADC_InitStructure = {0};
	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	// 关闭连续模式
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	// 右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	// 软件启动
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	// 独立模式
	ADC_InitStructure.ADC_NbrOfChannel = 1;	// 常规序列1个通道
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	// 关闭扫描模式
	
	ADC_Init(ADC1, &ADC_InitStructure);
	
	// #3. 初始化常规序列
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_13Cycles5);
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
	
	// #4. 闭合ADC的总开关
	ADC_Cmd(ADC1, ENABLE);
	
	// #5. ADC校准
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));

	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	
}

void App_OnMyBoardLED_Init(void)
{
	// PC13->开漏输出
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// 默认熄灭
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}

//
//	@简介：对串口1进行初始化
//	PA9 -Tx		PA10 -Rx
//	配置参数：115200  8数据位  1停止位  无校验位  
//
void My_USART1_Init(void)
{
	// # 开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_Initstructure = {0};
	// # PA9 Tx 复用推挽输出
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_Initstructure);
	// # PA10 Rx 浮空输入或上拉输入
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_Initstructure);
	
	// # 2.初始化USART1
	// # 开启USART1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	// # 初始化USART1
	USART_InitTypeDef USART1_InitStructure = {0};
	
	USART1_InitStructure.USART_BaudRate = 115200;
	USART1_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART1_InitStructure.USART_Parity = USART_Parity_No;
	USART1_InitStructure.USART_StopBits = USART_StopBits_1;
	
	USART_Init(USART1, &USART1_InitStructure);
	
	// # 打开USART的总开关
	USART_Cmd(USART1, ENABLE);
}
