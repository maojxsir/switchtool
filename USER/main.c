#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "pwm.h"


// #define DEBUG_LOG
#ifdef DEBUG_LOG
#define LOG(format, ...) printf(format, ##__VA_ARGS__);
#else
#define LOG(format, ...)
#endif
const char* VERSION = "1.20";

//max cmd para
#define MAX_CMD_CNT 50
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

CTLCMD recvcmd[ MAX_CMD_CNT ];  //CTRl+C (0x03)结束PWM执行
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
		"Usage: relay_k[num] [-para index] [total time] [high time]\n\r\n\r" \
		"  [num] 1,2,3,4.k1 and k2 use to USB,k3 k4 used to switch power by default\n\r" \
		"        if you want to change k1 and k2 to switch, connect P5,P6,P7 jumpper \n\r" \
		"        to P5,P6,P7 character side\n\r" \
		"  [-para] Support two paramter \n\r"	\
		"        -s k3 k4 two channels switch by same time\n\r" \
		"        -m specific multi pulses,max number 10\n\r" \
		"        -d two channels work on different frequence\n\r" \
		"        index number of the specific pulse\n\r" \
		"  [total time] Time of the period,10ms\n\r" \
		"  [high time] Time of the pulse,10ms\n\r\n\r" \
		"Others:\n\r" \
		"  - CTRL + C: stop \n\r" \
		"  - BackSpace: delete input\n\r\n\r" \
		"Examples:\n\r" \
		"  Switch USB,period 1000ms,active time 500ms,another word duty cycles 0.5:\n\r" \
		"      relay_k1 100 50\n\r" \
		"  Switch k3,period 1000ms,active time 300ms,duty cycles 0.3:\n\r" \
		"      relay_k3 100 30\n\r" \
		"  At the same time switch k3 k4,period 1000ms,active time 500ms,duty cycles 0.5:\n\r" \
		"      relay_k3 -s 100 50\n\r" \
		"  Specific multi pulses,\"_-__--___---\":\n\r" \
		"      relay_k3 -m 3 100 50 200 100 300 150\n\r" \
		"  K3 K4 two channles work at different time.\n\r" \
		"      relay_k3 -d 2000 100 k4 5000 100\n\r" \
	);

	uUartRecvFlag = 0;
	USART_RX_STA = 0;
	printf("\r\n>"); //echo back

}
int getcmdlen(char *pstr, char *pdest)
{
	int i = 0;
	while(*pstr != ' ' && *pstr != '\r')
	{
		i++;
		*pdest++ = *pstr++;
	}
	*pdest ='\0';
	return i;
}
void DoSubChannel(CTLCMD* pcmd)
{
	unsigned int totalB = 0, highB = 0;
	totalB = atoi((pcmd+1)->cmd);
	highB = atoi((pcmd+2)->cmd);
	if(strcmp(pcmd->cmd, "k2") == 0) //PB1
	{
		LOG("Line%d relay_k2 %d %d\n\r",__LINE__,totalB, highB);
		setTIM3CH4(totalB, highB);
		TIM_Cmd(TIM3, ENABLE);  // enable TIM3
	}
	else if (strcmp(pcmd->cmd, "k3") == 0) //PB6
	{
		LOG("Line%d relay_k3 %d %d\n\r",__LINE__,totalB, highB);
		setTIM4CH1(totalB, highB);
		TIM_Cmd(TIM4, ENABLE);  // enable TIM4
	}
	else if (strcmp(pcmd->cmd, "k4") == 0) //PA2
	{
		LOG("Line%d relay_k4 %d %d\n\r", __LINE__,totalB, highB);
		setTIM2CH3(totalB, highB);
		TIM_Cmd(TIM2, ENABLE);  // enable TIM2
	}
	else {
		printf("Line%d RROR:invalid paramter\n\r", __LINE__);
		showhelp();
		return;
	}
}

void DoDoubleChannelPulse(CTLCMD* pcmd)
{
	// relay_k2 -d 2000 700 k2 2000 1500

	NVIC_InitTypeDef NVIC_InitStructure;
	CTLCMD* pTmp = pcmd;
	unsigned int totalA = 0, highA = 0;

	totalA = atoi((pcmd+2)->cmd);
	highA = atoi((pcmd+3)->cmd);

	if (strcmp(pcmd->cmd, "relay_k2") ==0 ) //multi pulse PB1
	{
		setTIM3CH4(totalA, highA);
		TIM_Cmd(TIM3, ENABLE);
		DoSubChannel(pcmd+4);
	}
	else if (strcmp(pcmd->cmd, "relay_k3") ==0 )  //multi pulse PB6
	{
		setTIM4CH1(totalA, highA);
		TIM_Cmd(TIM4, ENABLE);
		DoSubChannel(pcmd+4);
	}
	else if (strcmp(pcmd->cmd, "relay_k4") ==0 ) //multi pulse PA2
	{
		setTIM2CH3(totalA, highA);
		TIM_Cmd(TIM4, ENABLE);
		DoSubChannel(pcmd+4);
	}
	else {
		printf("Line%d RROR:invalid paramter\n\r", __LINE__);
		showhelp();
		return ;
	}
}

void DoMutiPulse(CTLCMD *pcmd)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	CTLCMD* pTmp = pcmd;
	unsigned int total = 0, high = 0;
	PULSE *pulse = recvMultiPulse;
	int i=0;
	pcmd +=2;
	i = g_umultipulsenum = atoi(pcmd->cmd);
	g_umultipulsenumshadow = g_umultipulsenum ;
	if(i > MAX_PULSE_CNT)
	{
		LOG("ERROR:invalid paramter\n\r");
		showhelp();
		return ;
	}
	pcmd++;
	total = atoi(pcmd->cmd);
	pcmd++;
	high = atoi(pcmd->cmd);
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

	if (strcmp( pTmp->cmd, "relay_k1") == 0) {
		setTIM3CH3(total, high);
		TIM_ITConfig(TIM3,TIM_IT_CC3,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
		NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
		TIM_Cmd(TIM3, ENABLE);
	}
	else if (strcmp(pTmp->cmd, "relay_k2") ==0 ) //multi pulse PB1
	{
		setTIM3CH4(total, high);
		TIM_ITConfig(TIM3,TIM_IT_CC4,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
		NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
		TIM_Cmd(TIM3, ENABLE);
	}
	else if (strcmp(pTmp->cmd, "relay_k3") ==0 )  //multi pulse PB6
	{
		setTIM4CH1(total, high);

		TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
		NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
		TIM_Cmd(TIM4, ENABLE);
	}
	else if (strcmp(pTmp->cmd, "relay_k4") ==0 ) //multi pulse PA2
	{
		setTIM2CH3(total, high);

		TIM_ITConfig(TIM2,TIM_IT_CC3,ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
		NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
		TIM_Cmd(TIM2, ENABLE);
	}
	else {
		printf("Line%d RROR:invalid paramter\n\r", __LINE__);
		showhelp();
		return ;
	}
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
		if(cmdlen == 0)
			break;
		if( g_recvcmdnum >= MAX_CMD_CNT)
			break;
		g_recvcmdnum++;
		pcmd++;
		pSrcBuf += cmdlen;
		if((*pSrcBuf == '\r')
			||(*pSrcBuf == '\n')
			|| (*pSrcBuf == '\0'))
			break;
		else
			pSrcBuf++;
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
			//LOG("relay_k1 %d %d\n",total, high);
			setTIM3CH3(total, high);
			TIM_Cmd(TIM3, ENABLE);

		}
		else if(strcmp(pcmd->cmd, "relay_k2") == 0) //PB1
		{
			pcmd++;
			total = strtol(pcmd->cmd,NULL,10);
			pcmd++;
			high = strtol(pcmd->cmd,NULL,10);
			//LOG("relay_k2 %d %d\n",total, high);
			setTIM3CH4(total, high);
			TIM_Cmd(TIM3, ENABLE);
		}
		else if (strcmp(pcmd->cmd, "relay_k3") == 0) //PB6
		{
			pcmd++;
			total = strtol(pcmd->cmd,NULL,10);
			pcmd++;
			high = strtol(pcmd->cmd,NULL,10);
			//LOG("relay_k3 %d %d\n",total, high);
			setTIM4CH1(total, high);
			TIM_Cmd(TIM4, ENABLE);
		}
		else if (strcmp(pcmd->cmd, "relay_k4") == 0) //PA2
		{
			pcmd++;
			total = atoi(pcmd->cmd);
			pcmd++;
			high = atoi(pcmd->cmd);
			//LOG("relay_k4 %d %d\n",total, high);
			setTIM2CH3(total, high);
			TIM_Cmd(TIM2, ENABLE);
		}
		else
		{
			printf("Line%d RROR:invalid paramter\n\r", __LINE__);
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
			//LOG("%s -s %d %d\n",(pcmd-3)->cmd,total, high);
			setDoubleTIM(total, high);
		}
		else
		{
			printf("Line%d RROR:invalid paramter\n\r", __LINE__);
			showhelp();
		}
	}
	//////////////////////multi pulse
	else if((g_recvcmdnum > 4) && (strcmp((pcmd + 1)->cmd, "-m\0") == 0))  //
	{
		DoMutiPulse(pcmd);
	}
	else if((g_recvcmdnum > 4) && (strcmp( (pcmd + 1)->cmd, "-d\0") == 0)) {
		DoDoubleChannelPulse(pcmd);
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
	TIM_PWM_Init(9999);   //default 72MHz/(9999+1) = 100Hz
	printf("***     welcome to use switch tool     ***\r\n" \
				 "***    Versin:%s,Date:"__DATE__"   ***\r\n>" \
	,VERSION);

	while(1)
	{
		if (uUartRecvFlag == 1)
		{//start
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
			LOG("TIM2_TIM_FLAG_CC3 Handler\r\n");

			TIM2->CCR3 = pMultiPulse->pulse;
			TIM2->ARR = pMultiPulse->period;

			TIM_ITConfig(TIM2,TIM_IT_CC3,ENABLE);
			pMultiPulse++ ;
		}
		else
		{
			TIM_ClearITPendingBit(TIM2, TIM_FLAG_CC3  );
			LOG("TIM2_TIM_FLAG_CC3 Handler\r\n");
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
	LOG("TIM3_IRQHandler\r\n");
	if(TIM_GetFlagStatus(TIM3,TIM_FLAG_CC3)== SET )
	{
		if(--g_umultipulsenumshadow)  //k1
		{
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC3  );
			LOG("TIM3_TIM_FLAG_CC3 Handler\r\n");


			TIM3->CCR3 = pMultiPulse->pulse;
			TIM3->ARR = pMultiPulse->period;

			TIM_ITConfig(TIM3,TIM_IT_CC3,ENABLE);
			pMultiPulse++ ;
		}
		else
		{
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC4  );
			LOG("TIM3_TIM_FLAG_CC3 Handler\r\n");
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
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC4  );
			LOG("TIM3_TIM_FLAG_CC4 Handler\r\n");


			TIM3->CCR4 = pMultiPulse->pulse;
			TIM3->ARR = pMultiPulse->period;

			TIM_ITConfig(TIM3,TIM_IT_CC4,ENABLE);
			pMultiPulse++ ;
		}
		else
		{
			TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC4  );
			LOG("TIM3_TIM_FLAG_CC4 Handler\r\n");
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
	LOG("TIM4_IRQHandler\r\n");
	if(TIM_GetFlagStatus(TIM4,TIM_FLAG_CC1)== SET )
	{
		if(--g_umultipulsenumshadow)
		{
			TIM_ClearITPendingBit(TIM4, TIM_FLAG_CC1  );
			LOG("TIM4_TIM_FLAG_CC1 Handler\r\n");

			TIM4->CCR1 = pMultiPulse->pulse;
			TIM4->ARR = pMultiPulse->period;

			TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);
			pMultiPulse++ ;
		}
		else
		{
			TIM_ClearITPendingBit(TIM4, TIM_FLAG_CC1  );
			LOG("TIM4_TIM_FLAG_CC1 Handler\r\n");
			pMultiPulse = recvMultiPulse;
			g_umultipulsenumshadow = g_umultipulsenum;

			TIM4->CCR1 = pMultiPulse->pulse;
			TIM4->ARR = pMultiPulse->period;
			TIM_ITConfig(TIM4,TIM_IT_CC1,ENABLE);
			pMultiPulse++ ;
		}
	}
}