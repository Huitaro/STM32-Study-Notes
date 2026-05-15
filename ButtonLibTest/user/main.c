#include "stm32f10x.h"
#include "button.h"
#include "usart.h"

uint32_t cnt = 0;	// 计数器

Button_TypeDef button1;

void App_USART_Init(void);
void App_Button_Init(void);

void button_clicked_cb(uint8_t clicks);
void button_long_pressed_cb(uint8_t ticks);

int main(void)
{
	App_USART_Init();
//	My_USART_SendString(USART1, "Hello World!!\r\n");
	App_Button_Init();
	
	while(1)
	{
		My_Button_Proc(&button1);
	}
}

//
//	@简介：对USART进行初始化
//
void App_USART_Init(void)
{
	// #1. 完成PA9、PA10的初始化
	// PA9->Tx		PA10->Rx
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// #2. 完成USART1的初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	
	USART_Init(USART1, &USART_InitStructure);
	
	// #3. 打开USART1的总开关
	USART_Cmd(USART1, ENABLE);
}

//
//	@简介：对按键进行初始化
//
void App_Button_Init(void)
{
	Button_InitTypeDef ButtonInitStructure;
	
	ButtonInitStructure.GPIOx = GPIOA;
	ButtonInitStructure.GPIO_Pin = GPIO_Pin_0;
	ButtonInitStructure.LongPressTime = 500;	// 500ms触发
	ButtonInitStructure.LongPressTickInterval = 0;
	ButtonInitStructure.ClickInterval = 0;
	ButtonInitStructure.button_pressed_cb = 0;
	ButtonInitStructure.button_released_cb = 0;
	ButtonInitStructure.button_clicked_cb = button_clicked_cb;
	ButtonInitStructure.button_long_pressed_cb = button_long_pressed_cb;
	
	My_Button_Init(&button1, &ButtonInitStructure);
}

//
//	@简介：点击1次，计数器加1；点击2次，计数器清零
//
void button_clicked_cb(uint8_t clicks)
{
	if(clicks == 1)
	{
		cnt++;
		My_USART_Printf(USART1, "%d", cnt);
	}
	else if(clicks == 2)
	{
		cnt = 0;
		My_USART_Printf(USART1, "%d", cnt);
	}
}

//
//	@简介：长按下计数器连续+1
//
void button_long_pressed_cb(uint8_t ticks)
{
	cnt++;
	My_USART_Printf(USART1, "%d", cnt);
}
