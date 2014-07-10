#include "pwm.h"


//TMIx PWM�����ʼ�� GPIO��ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM_PWM_Init(u16 arr)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB , ENABLE);  //ʹ��GPIOA GPIOB����ʱ��ʹ��
	                                                                   
   //���ø�����Ϊ�����������,���TIM1 CH1��PWM���岨��	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2--TMI2-CH3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//PB0--TMI3-CH3, PB1--TMI3-CH4, PB6--TMI4-CH1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  				//�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOA, GPIO_Pin_2);
//	GPIO_SetBits(GPIOA, GPIO_Pin_0);
//	GPIO_SetBits(GPIOA, GPIO_Pin_1);
//	GPIO_SetBits(GPIOA, GPIO_Pin_6);
	
	
	///////////////////////////////////////
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);
 	
//	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80K
//	TIM_TimeBaseStructure.TIM_Prescaler = PSCALER; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  4��Ƶ
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
//	
//	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIM2��ʱ�������λ
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIM3��ʱ�������λ
//	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIM4��ʱ�������λ
//	 
//	 
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //�Ƚ����ʹ��
//	TIM_OCInitStructure.TIM_Pulse = 500; //���ô�װ�벶��ȽϼĴ���������ֵ
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
//	
//	
//	//PA2
//	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM2-CH3 
//	//PB0
//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM3-CH3	
//	//PB1
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM3-CH4
//	//PB6
//	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM4-CH1
	

	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);  //CH3Ԥװ��ʹ��	 
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);  //CH3Ԥװ��ʹ��	
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);  //CH4Ԥװ��ʹ��	
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  //CH1Ԥװ��ʹ��	


	TIM_ARRPreloadConfig(TIM2, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	TIM_ARRPreloadConfig(TIM3, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	TIM_ARRPreloadConfig(TIM4, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���

	TIM_Cmd(TIM2, DISABLE);  //ʹ��TIM2
	TIM_Cmd(TIM3, DISABLE);  //ʹ��TIM3
	TIM_Cmd(TIM4, DISABLE);  //ʹ��TIM4
 
}

//K1-PB0
void setTIM3CH3(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
		TIM_TimeBaseStructure.TIM_Period = total - 1; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  72��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM2������

	TIM_OCInitStructure.TIM_Pulse = high; //����ռ�ձ�
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	//PB0
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM3-CH3 
	
	//TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
}
//K2-PB1
void setTIM3CH4(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
		TIM_TimeBaseStructure.TIM_Period = total - 1; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  72��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM2������


	TIM_OCInitStructure.TIM_Pulse = high; //����ռ�ձ�
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	//PB1
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM3-CH4 
	
	//TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
}
//K3-PB6
void setTIM4CH1(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	TIM_TimeBaseStructure.TIM_Period = total - 1; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  72��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM2������

	TIM_OCInitStructure.TIM_Pulse = high; //����ռ�ձ�
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	//PB6
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM4-CH1 
	
	//TIM_Cmd(TIM4, ENABLE);  //ʹ��TIM4
}

//K4-PA2
void setTIM2CH3(unsigned int total, unsigned int high)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	TIM_TimeBaseStructure.TIM_Period = total - 1; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  72��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM2������

	TIM_OCInitStructure.TIM_Pulse = high; //����ռ�ձ�
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	//PA2
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM2-CH3 
	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIM4
}

//Stop all of the TIMx
void StopTIMxALL(void)
{
//	TIM_OCInitTypeDef  TIM_OCInitStructure;
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //�Ƚ����ʹ��	
//	//PA2
//	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM2-CH3 
//	//PB0
//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM3-CH3	
//	//PB1
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM3-CH4
//	//PB6
//	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM4-CH1
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
	
	TIM_TimeBaseStructure.TIM_Period = total - 1; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =PSCALER; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  72��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM2������

	TIM_OCInitStructure.TIM_Pulse = high; //����ռ�ձ�
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	//PA2
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM2-CH3 
	//PB6
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIM4-CH1 
	
	TIM_Cmd(TIM4, ENABLE);  //ʹ��TIM4
	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIM2
	
}
