#include "stm32f10x.h"
#include "delay.h"
#include "math.h"

#define PI 3.1415926

void App_PWM_Init(void);

int main(void)
{
	App_PWM_Init();
	
	while(1)
	{
		float t = GetTick() * 1.0e-3f;	// 获取系统当前时间，换算成单位s
		float duty = 0.5 * (sin(2 * PI * t) + 1.0);		// 计算占空比
		
		uint16_t ccr1 = duty * 1000;	// 比例转换[0,1] -> [0,999]
		TIM_SetCompare1(TIM1, ccr1);
	}
}

void App_PWM_Init(void)
{
	// #1. 初始化GPIO的引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	// PA8	AF_PP
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// PB13	 AF_PP
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// #2. 设置时基单元的参数
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	//打开TIM1的时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	/* 
			1/(72MHz/(71+1)) × (999+1) = 1ms 
	*/
	TIM_TimeBaseInitStructure.TIM_Prescaler = 71;
	TIM_TimeBaseInitStructure.TIM_Period = 999;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	
	
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	
	TIM_ARRPreloadConfig(TIM1, ENABLE);	//闭合自动重装载ARR的寄存器预加载开关
	
	TIM_Cmd(TIM1, ENABLE);	// 闭合时基单元的开关
	
	// #3. 配置输出比较寄存器
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	
	// 闭合MOE的开关
	TIM_CtrlPWMOutputs(TIM1, ENABLE);		
	TIM_CCPreloadControl(TIM1, ENABLE);
}
