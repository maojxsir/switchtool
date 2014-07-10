#include "pwm.h"


//TMIx PWM输出初始化 GPIO初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM_PWM_Init(u16 arr)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB , ENABLE);  //使能GPIOA GPIOB外设时钟使能
	                                                                   
   //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2--TMI2-CH3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//PB0--TMI3-CH3, PB1--TMI3-CH4, PB6--TMI4-CH1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  				//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOA, GPIO_Pin_2);
//	GPIO_SetBits(GPIOA, GPIO_Pin_0);
//	GPIO_SetBits(GPIOA, GPIO_Pin_1);
//	GPIO_SetBits(GPIOA, GPIO_Pin_6);
	
	
	///////////////////////////////////////
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);
 	
//	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
//	TIM_TimeBaseStructure.TIM_Prescaler = PSCALER; //设置用来作为TIMx时钟频率除数的预分频值  4分频
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
//	
//	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIM2的时间基数单位
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIM3的时间基数单位
//	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIM4的时间基数单位
//	 
//	 
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //比较输出使能
//	TIM_OCInitStructure.TIM_Pulse = 500; //设置待装入捕获比较寄存器的脉冲值
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
//	
//	
//	//PA2
//	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM2-CH3 
//	//PB0
//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM3-CH3	
//	//PB1
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM3-CH4
//	//PB6
//	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM4-CH1
	

	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);  //CH3预装载使能	 
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);  //CH3预装载使能	
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);  //CH4预装载使能	
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  //CH1预装载使能	


	TIM_ARRPreloadConfig(TIM2, ENABLE); //使能TIMx在ARR上的预装载寄存器
	TIM_ARRPreloadConfig(TIM3, ENABLE); //使能TIMx在ARR上的预装载寄存器
	TIM_ARRPreloadConfig(TIM4, ENABLE); //使能TIMx在ARR上的预装载寄存器

	TIM_Cmd(TIM2, DISABLE);  //使能TIM2
	TIM_Cmd(TIM3, DISABLE);  //使能TIM3
	TIM_Cmd(TIM4, DISABLE);  //使能TIM4
 
}

//K1-PB0
void setTIM3CH3(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
		TIM_TimeBaseStructure.TIM_Period = total - 1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //设置用来作为TIMx时钟频率除数的预分频值  72分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //设置TIM2的周期

	TIM_OCInitStructure.TIM_Pulse = high; //设置占空比
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	//PB0
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM3-CH3 
	
	//TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}
//K2-PB1
void setTIM3CH4(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
		TIM_TimeBaseStructure.TIM_Period = total - 1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //设置用来作为TIMx时钟频率除数的预分频值  72分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //设置TIM2的周期


	TIM_OCInitStructure.TIM_Pulse = high; //设置占空比
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	//PB1
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM3-CH4 
	
	//TIM_Cmd(TIM3, ENABLE);  //使能TIM3
}
//K3-PB6
void setTIM4CH1(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	TIM_TimeBaseStructure.TIM_Period = total - 1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //设置用来作为TIMx时钟频率除数的预分频值  72分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //设置TIM2的周期

	TIM_OCInitStructure.TIM_Pulse = high; //设置占空比
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	//PB6
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM4-CH1 
	
	//TIM_Cmd(TIM4, ENABLE);  //使能TIM4
}

//K4-PA2
void setTIM2CH3(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	TIM_TimeBaseStructure.TIM_Period = total - 1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //设置用来作为TIMx时钟频率除数的预分频值  72分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //设置TIM2的周期

	TIM_OCInitStructure.TIM_Pulse = high; //设置占空比
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	//PA2
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM2-CH3 
	TIM_Cmd(TIM2, ENABLE);  //使能TIM4
}

//Stop all of the TIMx
void StopTIMxALL(void)
{
//	TIM_OCInitTypeDef  TIM_OCInitStructure;
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //比较输出使能	
//	//PA2
//	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM2-CH3 
//	//PB0
//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM3-CH3	
//	//PB1
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM3-CH4
//	//PB6
//	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM4-CH1
//	
//	TIM_Cmd(TIM2, DISABLE);  //stop TIM2
//	TIM_Cmd(TIM3, DISABLE);  //stop TIM3
//	TIM_Cmd(TIM4, DISABLE);  //stop TIM4
	TIM_DeInit(TIM2);
	TIM_DeInit(TIM3);
	TIM_DeInit(TIM4);
}

//K3-PB6 K4-PA2
void setDoubleTIM(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	TIM_TimeBaseStructure.TIM_Period = total - 1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //设置用来作为TIMx时钟频率除数的预分频值  72分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //设置TIM2的周期

	TIM_OCInitStructure.TIM_Pulse = high; //设置占空比
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	//PA2
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM2-CH3 
	//PB6
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIM4-CH1 
	
	TIM_Cmd(TIM4, ENABLE);  //使能TIM4
	TIM_Cmd(TIM2, ENABLE);  //使能TIM2
	
}
