#include "stm32f10x.h"
#include "stm32f10x_it.h" 
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "pwm.h"

//max cmd para
#define MAX_CMD_CNT 30
#define MAX_PULSE_CNT 10
typedef struct{
	int period;
	int pulse;
}PULSE;
PULSE recvMultiPulse[10];
PULSE *pMultiPulse = NULL;

typedef struct{
	char cmd[10];	
}CTLCMD; 

CTLCMD recvcmd[ MAX_CMD_CNT ];  //max cmd para
int g_recvcmdnum = 0;


extern u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
extern u16 USART_RX_STA;       //接收状态标记

u8 uUartRecvFlag = 0;
u8 g_uExitPwmFlag =0; //CTRl+C (0x03)结束PWM执行
u8 g_umultipulsenum = 0;
u8 g_umultipulsenumshadow = 0;


void Delay(u32 count)
{
	u32 i=0;
	for(;i<count;i++);

}

void showhelp(void)
{
	printf(
		"Usage: relay_k[num] [-para index] [total time] [high time]\n" \
		"  [num] 1,2,3,4.k1 and k2 use to USB,k3 k4 used to switch power by default\n" \
		"        if you want to change k1 and k2 to switch,P5,P6,P7jumpper \n" \
		"        connect to P5,P6,P7 character side\n" \
		"  [-para] support two paramter \n"	\
		"        -s k3 k4 two channels switch by smae time\n" \
		"        -m specific multi pulses,max number 10\n" \
		"        index number of the specific pulse\n" \
		"  [total time] time of the period,1ms\n" \
		"  [high time] time of the pulse,1ms\n" \
		"Others:\n" \
		"  - CTRL + C: stop \n" \
		"  - BackSpace: delete input\n" \
		"Examples:\n" \
		"  switch USB,period 1000ms,active time 500ms,another word duty cycles 0.5:\n" \
		"      relay_k1 1000 500\n" \
		"  switch k3,period 1000ms,active time 300ms,duty cycles 0.3:\n" \
		"      relay_k3 1000 300\n" \
		"  at the same time switch k3 k4,period 1000ms,active time 500ms,duty cycles 0.5:\n" \
		"      relay_k3 -s 1000 500\n" \
		"  specific multi pulses,\"_-__--___---\":\n" \
		"      relay_k3 -m 3 1000 500 2000 1000 3000 1500\n" \
	);
	uUartRecvFlag = 0;
	USART_RX_STA = 0;
	printf("\r\n>"); //echo back

}
int getcmdlen(char *pstr, char *pdest)
{
	int i = 0;
	while(*pstr != ' ' & *pstr != '\r' & *pstr != '\n')
	{	
		i++;
		*pdest++ = *pstr++;
	}
	*pdest ='\0';
	return i;	
}

int parsecmd(void)
{ 
	NVIC_InitTypeDef NVIC_InitStructure;
	int recvlen = 0;
	unsigned int total = 0, high = 0;
	char * pSrcBuf = (char*)USART_RX_BUF;
	int cmdlen = 0;
	CTLCMD *pcmd = recvcmd;
	PULSE *pulse = recvMultiPulse;
	int i=0;
	g_recvcmdnum = 0;
	memset(&recvcmd, 0, sizeof(recvcmd));
	
	recvlen = strlen( (const char *)USART_RX_BUF );
	if (recvlen < sizeof("relay_x"))
	{
		showhelp();
		return -1;
	}
	while(1)
	{
		cmdlen = getcmdlen( pSrcBuf, pcmd->cmd );
		if(cmdlen == 0 )
			break;
		if( g_recvcmdnum >= MAX_CMD_CNT)
			break;
		g_recvcmdnum++;
		pcmd++;
		pSrcBuf += cmdlen + 1;
	}
	
	/////relay_k1 1000 500
	pcmd = recvcmd;
	if( g_recvcmdnum == 3)  //
	{
		if (strcmp( pcmd->cmd, "relay_k1") == 0) //PB0
		{		
			pcmd++;
			total = strtol(pcmd->cmd, NULL, 10);
			pcmd++;
			high = strtol(pcmd->cmd, NULL, 10);
			//printf("relay_k1 %d %d\n",total, high);
			setTIM3CH3(total, high);
			TIM_Cmd(TIM3, ENABLE);  //使能TIM3			
			
		}
		else if(strcmp(pcmd->cmd, "relay_k2") == 0) //PB1
		{
			pcmd++;
			total = strtol(pcmd->cmd,NULL,10);
			pcmd++;
			high = strtol(pcmd->cmd,NULL,10);
			//printf("relay_k2 %d %d\n",total, high);
			setTIM3CH4(total, high);
			TIM_Cmd(TIM3, ENABLE);  //使能TIM3
		}	
		else if (strcmp(pcmd->cmd, "relay_k3") == 0) //PB6
		{
			pcmd++;
			total = strtol(pcmd->cmd,NULL,10);
			pcmd++;
			high = strtol(pcmd->cmd,NULL,10);
			//printf("relay_k3 %d %d\n",total, high);
			setTIM4CH1(total, high);
			TIM_Cmd(TIM4, ENABLE);  //使能TIM4
		}	
		else if (strcmp(pcmd->cmd, "relay_k4") == 0) //PA2
		{
			pcmd++;
			total = atoi(pcmd->cmd);
			pcmd++;
			high = atoi(pcmd->cmd);
			//printf("relay_k4 %d %d\n",total, high);
			setTIM2CH3(total, high);
			TIM_Cmd(TIM2, ENABLE);  //使能TIM2
		}
		else 
		{
			printf("ERROR:invalid paramter\n");
			showhelp();
			return -1;
		}	
	}
	else if( g_recvcmdnum == 4)  //
	{
		if (((strcmp( pcmd->cmd, "relay_k3") == 0) \
			||(strcmp( pcmd->cmd, "relay_k4") == 0))\
			&(strcmp( (++pcmd)->cmd, "-s\0") == 0)
			) //PB0
		{		
			pcmd++;
			total = strtol(pcmd->cmd,NULL,10);
			pcmd++;
			high = strtol(pcmd->cmd,NULL,10);
			//printf("%s -s %d %d\n",(pcmd-3)->cmd,total, high);
			setDoubleTIM(total, high);		
		}
		else
		{
			printf("ERROR:invalid paramter\n");
			showhelp();
		}
	}	
	//////////////////////multi pulse
	else if( g_recvcmdnum > 4)  //
	{
		if ((strcmp( pcmd->cmd, "relay_k1") == 0) \
			&(strcmp( (pcmd + 1)->cmd, "-m\0") == 0)
			) //multi pulse PB0
		{		
			pcmd +=2;
			i = g_umultipulsenum = atoi(pcmd->cmd);			
			g_umultipulsenumshadow = g_umultipulsenum ;
			if(i > MAX_PULSE_CNT)
			{
				printf("ERROR:invalid paramter\n");
				showhelp();
				return -1;
			}
			pcmd++;
			total = atoi(pcmd->cmd);
			pcmd++;
			high = atoi(pcmd->cmd);
			//printf("relay_k1 %d %d\n",total, high);
			pulse->period = total ;
			pulse->pulse = high ;
			pulse++;
			i--;
			while(i--)
			{
				pcmd++;
				pulse->period = atoi(pcmd->cmd);
				pcmd++;
				pulse->pulse = atoi(pcmd->cmd);
				pulse ++;
			}
			setTIM3CH3(total, high);
			
			TIM_ITConfig(TIM3,TIM_IT_CC3,ENABLE);
			NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
			NVIC_Init(&NVIC_InitStructure);  //初始化外设NVIC寄存器
			TIM_Cmd(TIM3, ENABLE);  //使能TIM3
		}
		else if((strcmp(pcmd->cmd, "relay_k2") ==0 ) \
				&(strcmp( (pcmd + 1)->cmd, "-m\0") == 0)
				) //multi pulse PB1
		{
			pcmd +=2;
			i = g_umultipulsenum = atoi(pcmd->cmd);			
			g_umultipulsenumshadow = g_umultipulsenum ;
			if(i > MAX_PULSE_CNT)
			{
				printf("ERROR:invalid paramter\n");
				showhelp();
				return -1;
			}
			pcmd++;
			total = atoi(pcmd->cmd);
			pcmd++;
			high = atoi(pcmd->cmd);
			//printf("relay_k2 %d %d\n",total, high);
			pulse->period = total ;
			pulse->pulse = high ;
			pulse++;
			i--;
			while(i--)
			{
				pcmd++;
				pulse->period = atoi(pcmd->cmd);
				pcmd++;
				pulse->pulse = atoi(pcmd->cmd);
				pulse ++;
			}
			setTIM3CH4(total, high);
			
			TIM_ITConfig(TIM3,TIM_IT_CC4,ENABLE);
			NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
			NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
			TIM_Cmd(TIM3, ENABLE);  //使能TIM3
		}	
		else if ((strcmp(pcmd->cmd, "relay_k3") ==0 ) \
				&(strcmp( (pcmd + 1)->cmd, "-m\0") == 0)
				)  //multi pulse PB6
		{
			pcmd +=2;
			i = g_umultipulsenum = atoi(pcmd->cmd);			
			g_umultipulsenumshadow = g_umultipulsenum ;
			if(i > MAX_PULSE_CNT)
			{
				printf("ERROR:invalid paramter\n");
				showhelp();
				return -1;
			}
			pcmd++;
			total = atoi(pcmd->cmd);
			pcmd++;
			high = atoi(pcmd->cmd);
			//printf("relay_k3 %d %d\n",total, high);
			pulse->period = total ;
			pulse->pulse = high ;
			pulse++;
			i--;
			while(i--)
			{
				pcmd++;
				pulse->period = atoi(pcmd->cmd);
				pcmd++;
				pulse->pulse = atoi(pcmd->cmd);
				pulse ++;
			}
			setTIM4CH1(total, high);
			
			TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);
			NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
			NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
			TIM_Cmd(TIM4, ENABLE);  //使能TIM4
		}	
		else if ((strcmp(pcmd->cmd, "relay_k4") ==0 ) \
				&(strcmp( (pcmd + 1)->cmd, "-m\0") == 0)
				)  //multi pulse PA2
		{
			pcmd +=2;
			i = g_umultipulsenum = atoi(pcmd->cmd);			
			g_umultipulsenumshadow = g_umultipulsenum ;
			if(i > MAX_PULSE_CNT)
			{
				printf("ERROR:invalid paramter\n");
				showhelp();
				return -1;
			}
			pcmd++;
			total = atoi(pcmd->cmd);
			pcmd++;
			high = atoi(pcmd->cmd);
			//printf("relay_k4 %d %d\n",total, high);
			pulse->period = total ;
			pulse->pulse = high ;
			pulse++;
			i--;
			while(i--)
			{
				pcmd++;
				pulse->period = atoi(pcmd->cmd);
				pcmd++;
				pulse->pulse = atoi(pcmd->cmd);
				pulse ++;
			}
			setTIM2CH3(total, high);
			
			TIM_ITConfig(TIM2,TIM_IT_CC3,ENABLE);
			NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
			NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
			TIM_Cmd(TIM2, ENABLE);  //使能TIM2
		}
		else 
		{
			printf("ERROR:invalid paramter\n");
			showhelp();
			return -1;
		}	
	}
	else
	{	
		showhelp();
		return -1;
	}
	return 0 ;
}

//系统时钟初始化函数
//pll:选择的倍频数，从2开始，最大值为16		 
void Stm32_Clock_Init(u8 PLL)
{
	unsigned char temp=0;   
	//MYRCC_DeInit();		  //复位并配置向量表
 	RCC->CR|=0x00010000;  //外部高速时钟使能HSEON
	while(!(RCC->CR>>17));//等待外部时钟就绪
	RCC->CFGR=0X00000700; //APB1=DIV4;APB2=DIV1;AHB=DIV2;
	PLL-=2;//抵消2个单位
	RCC->CFGR|=PLL<<18;   //设置PLL值 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON 
	FLASH->ACR|=0x32;	  //FLASH 2个延时周期

	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//等待PLL锁定
	RCC->CFGR|=0x00000002;//PLL作为系统时钟	 
	while(temp!=0x02)     //等待PLL作为系统时钟设置成功
	{   
		temp=RCC->CFGR>>2;
		temp&=0x03;
	}    
}		    

int main(void)
{	
	RCC_PCLK1Config(RCC_HCLK_Div4);
	// uart init
	uart_init(115200);	
	// TIM init and GPIO init
	TIM_PWM_Init(999);   //default 72MHz/(999+1) = 1KHz
	printf("***    welcome to use switch tool    ***\r\n>"); 

	while(1)
	{
		
		if (uUartRecvFlag == 1)
		{//start
			//printf((char *)USART_RX_BUF);
			parsecmd();
			pMultiPulse = recvMultiPulse;
			pMultiPulse++ ;
			uUartRecvFlag = 0;
			USART_RX_STA = 0;

		}
		if( g_uExitPwmFlag )
		{ //stop
			StopTIMxALL();
			g_uExitPwmFlag = 0;
			USART_RX_STA = 0;
			memset(&USART_RX_STA, 0 , sizeof(USART_RX_STA));
			pMultiPulse = recvMultiPulse;
					
			TIM_ITConfig(TIM2,TIM_IT_CC3,DISABLE);
			TIM_ITConfig(TIM3,TIM_IT_CC3,DISABLE);
			TIM_ITConfig(TIM3,TIM_IT_CC4,DISABLE);
			TIM_ITConfig(TIM4,TIM_IT_CC1,DISABLE);
		}
	}
}


void TIM2_IRQHandler(void)
{
	if(TIM_GetFlagStatus(TIM2,TIM_FLAG_CC3)== SET )
	{
		if(--g_umultipulsenumshadow)
		{
			TIM_ClearITPendingBit(TIM2, TIM_FLAG_CC3  );  //清除TIMx的中断待处理位:TIM 中断源
			printf("TIM2_TIM_FLAG_CC3 Handler\r\n");

			TIM2->CCR3 = pMultiPulse->pulse; 
			TIM2->ARR = pMultiPulse->period; 
			
			TIM_ITConfig(TIM2,TIM_IT_CC3,ENABLE);
			pMultiPulse++ ;
		}	
		else
		{
			TIM_ClearITPendingBit(TIM2, TIM_FLAG_CC3  );  //清除TIMx的中断待处理位:TIM 中断源
			printf("TIM2_TIM_FLAG_CC3 Handler\r\n");
			pMultiPulse = recvMultiPulse;
			g_umultipulsenumshadow = g_umultipulsenum;
			
			TIM2->CCR3 = pMultiPulse->pulse; 
			TIM2->ARR = pMultiPulse->period; 
			TIM_ITConfig(TIM2,TIM_IT_CC3,ENABLE);
			pMultiPulse++ ;
		}	
	}
}
void TIM3_IRQHandler(void)
{
	printf("TIM3_IRQHandler\r\n");
	if(TIM_GetFlagStatus(TIM3,TIM_FLAG_CC3)== SET )
	{
		if(--g_umultipulsenumshadow)  //k1
		{
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC3  );  //清除TIMx的中断待处理位:TIM 中断源
			printf("TIM3_TIM_FLAG_CC3 Handler\r\n");


			TIM3->CCR3 = pMultiPulse->pulse; 
			TIM3->ARR = pMultiPulse->period; 
			
			TIM_ITConfig(TIM3,TIM_IT_CC3,ENABLE);//开中断
			pMultiPulse++ ;
		}	
		else
		{
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC4  );  //清除TIMx的中断待处理位:TIM 中断源
			printf("TIM3_TIM_FLAG_CC3 Handler\r\n");
			pMultiPulse = recvMultiPulse;
			g_umultipulsenumshadow = g_umultipulsenum;
			
			TIM3->CCR4 = pMultiPulse->pulse; 
			TIM3->ARR = pMultiPulse->period; 
			TIM_ITConfig(TIM3,TIM_IT_CC4,ENABLE);
			pMultiPulse++ ;
		}	
	}
	else if(TIM_GetFlagStatus(TIM3,TIM_FLAG_CC4)== SET )
	{
		if(--g_umultipulsenumshadow)
		{
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC4  );  //清除TIMx的中断待处理位:TIM 中断源
			printf("TIM3_TIM_FLAG_CC4 Handler\r\n");


			TIM3->CCR4 = pMultiPulse->pulse; 
			TIM3->ARR = pMultiPulse->period; 
			
			TIM_ITConfig(TIM3,TIM_IT_CC4,ENABLE);
			pMultiPulse++ ;
		}	
		else
		{
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC4  );  //清除TIMx的中断待处理位:TIM 中断源
			printf("TIM3_TIM_FLAG_CC4 Handler\r\n");
			pMultiPulse = recvMultiPulse;
			g_umultipulsenumshadow = g_umultipulsenum;
			
			TIM3->CCR4 = pMultiPulse->pulse; 
			TIM3->ARR = pMultiPulse->period; 
			TIM_ITConfig(TIM3,TIM_IT_CC4,ENABLE);
			pMultiPulse++ ;
		}	
	}
		
}
void TIM4_IRQHandler(void)
{
	printf("TIM4_IRQHandler\r\n");
	if(TIM_GetFlagStatus(TIM4,TIM_FLAG_CC1)== SET )
	{
		if(--g_umultipulsenumshadow)
		{
			TIM_ClearITPendingBit(TIM4, TIM_FLAG_CC1  );  //清除TIMx的中断待处理位:TIM 中断源
			printf("TIM4_TIM_FLAG_CC1 Handler\r\n");

			TIM4->CCR1 = pMultiPulse->pulse; 
			TIM4->ARR = pMultiPulse->period; 
			
			TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);
			pMultiPulse++ ;
		}	
		else
		{
			TIM_ClearITPendingBit(TIM4, TIM_FLAG_CC1  );  //清除TIMx的中断待处理位:TIM 中断源
			printf("TIM4_TIM_FLAG_CC1 Handler\r\n");
			pMultiPulse = recvMultiPulse;
			g_umultipulsenumshadow = g_umultipulsenum;
			
			TIM4->CCR1 = pMultiPulse->pulse; 
			TIM4->ARR = pMultiPulse->period; 
			TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);
			pMultiPulse++ ;
		}	
	}
}
