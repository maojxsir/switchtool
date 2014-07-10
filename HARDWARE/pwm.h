#ifndef __PWM_H
#define __PWM_H
#include "stm32f10x_tim.h"

#define PSCALER 36000-1  //1ms

void TIM_PWM_Init(u16 arr);
void setTIM2CH3(unsigned int total, unsigned int high);
void setTIM3CH3(unsigned int total, unsigned int high);
void setTIM3CH4(unsigned int total, unsigned int high);
void setTIM4CH1(unsigned int total, unsigned int high);
void setDoubleTIM(unsigned int total, unsigned int high);
//Stop all of the TIMx
void StopTIMxALL(void);
#endif
