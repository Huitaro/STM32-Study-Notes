#include "stm32f10x.h"
#include "usart.h"

void App_USART1_Init(void);
void App_TIM1_Init(void);
void App_ADC1_Init(void);

int main(void)
{
	App_USART1_Init();
	App_TIM1_Init();
	App_ADC1_Init();
	
//	My_USART_SendString(USART1, "Hello World!!\r\n");
	
	while(1)
	{
		// #1. 等待JEOC标志位置1
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC) == RESET);
		
		// #2. 清除JEOC标志位
		ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);
		
		// #3. 获取jdr的值
		uint16_t jdr1 = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
		
		// #4. 转换成电压值并串口输出
		float voltage = jdr1 * (3.3f / 4095.0f);
		My_USART_Printf(USART1, "%.3f\n", voltage);
	}
}

void App_USART1_Init(void)
{
	// #1. 初始化GPIO引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	// PA9->Tx		AF_PP
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// PA10->Rx		IPU
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// #2. 初始化串口
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitTypeDef USART_InitStructure = {0};
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART1, &USART_InitStructure);
	
	// #3. 闭合USART1的总开关
	USART_Cmd(USART1, ENABLE);
}

void App_TIM1_Init(void)
{
	// #1. 打开定时器1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	// #2. 初始化时基单元
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
	
	TIM_TimeBaseInitStructure.TIM_Prescaler = 71;
	TIM_TimeBaseInitStructure.TIM_Period = 999;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	
	// #3. 配置触发输出
	TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
	
	// #4. 闭合时基单元的总开关
	TIM_Cmd(TIM1, ENABLE);
}

void App_ADC1_Init(void)
{
	// #1. 初始化GPIO引脚
	// PA0->AIN
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructrue = {0};
	
	GPIO_InitStructrue.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_AIN;
	
	GPIO_Init(GPIOA, &GPIO_InitStructrue);
	
	// #2. 配置ADC的时钟分频
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	// #3. 打开ADC1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	// #4. 配置ADC的相关参数
	ADC_InitTypeDef ADC_InitStructure = {0};
	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // 右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	
	ADC_Init(ADC1, &ADC_InitStructure);
	
	// #5. 配置注入序列
	ADC_InjectedSequencerLengthConfig(ADC1, 1);
	
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_13Cycles5);
	
	ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_TRGO);
	
	ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);

	// #6. 闭合ADC的总开关
	ADC_Cmd(ADC1, ENABLE);
}
