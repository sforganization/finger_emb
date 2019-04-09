
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "usart2.h"
#include "usart.h"
#include "as608.h"
#include "remote.h"
#include "key.h"
#include "beep.h"

#include "timer.h"

#define _MAININC_
#include "SysComment.h"
#undef _MAININC_
#define usart2_baund			57600//串口2波特率，根据指纹模块波特率更改（注意：指纹模块默认57600）



void SysTickTask(void);


void ledinit(void) //DEBUG 用

{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_13);
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void LockGPIOInit(void)
{
	/* 定义IO硬件初始化结构体变量 */
	GPIO_InitTypeDef GPIO_InitStructure;

	LOCK_RCC_CLOCKCMD(LOCK_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LOCK_A_PIN | LOCK_B_PIN | LOCK_C_PIN | LOCK_D_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LOCK_GPIO, &GPIO_InitStructure);

	GPIO_ResetBits(LOCK_GPIO, LOCK_A_PIN | LOCK_B_PIN | LOCK_C_PIN | LOCK_D_PIN);
}


/*******************************************************************************
* 名称: 
* 功能: 零点检测io
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Moto_ZeroGPIOInit(void)
{
	/* 定义IO硬件初始化结构体变量 */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能(开启)KEY1引脚对应IO端口时钟 */
	RCC_APB2PeriphClockCmd(LOCK_ZERO_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LOCK_ZERO_A_PIN | LOCK_ZERO_B_PIN | LOCK_ZERO_C_PIN | LOCK_ZERO_D_PIN;
	;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入

	GPIO_Init(LOCK_ZERO_GPIO, &GPIO_InitStructure);
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void PwmGPIOInit(void)
{
	/* 定义IO硬件初始化结构体变量 */
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(PWM_RCC_CLOCKGPIO, ENABLE); //使能B端口时钟
	GPIO_InitStructure.GPIO_Pin =
		 PWM_A_PIN_P | PWM_A_PIN_N | PWM_B_PIN_P | PWM_B_PIN_N | PWM_C_PIN_P | PWM_C_PIN_N | PWM_D_PIN_P | PWM_D_PIN_N;

	//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
	GPIO_Init(PWM_GPIO, &GPIO_InitStructure);
	GPIO_ResetBits(PWM_GPIO, 
		PWM_A_PIN_P | PWM_A_PIN_N | PWM_B_PIN_P | PWM_B_PIN_N | PWM_C_PIN_P | PWM_C_PIN_N | PWM_D_PIN_P | PWM_D_PIN_N);
}


int main(void)
{
	delay_init();									//延时函数初始化	  
	NVIC_Configuration();							//设置NVIC中断分组2:2位抢占优先级，2位响应优先级 	LED_Init();				 

	//uart_init(115200);								//初始化串口1波特率为115200，用于支持USMART	 打印as608用到
	usart2_init(usart2_baund);						//初始化串口2,用于与指纹模块通讯

	//PS_StaGPIO_Init();								//初始化FR读状态引脚  识别到指纹按压输出高电平，因为复用关系初始化动作放到KEY_GPIO_Init里面
	LockGPIOInit();
	Moto_ZeroGPIOInit();
	PwmGPIOInit();

	//BEEP_GPIO_Init();
	//ledinit();										//Debug 检测时钟是否正确
	OLED_Init();									//初始化OLED  
	KEY_GPIO_Init();								//其他IO初始化要放在这个前面
	OLED_Clear();
	SysInit();
	ShowLaunch();
	Remote_Init();
	GENERAL_TIMx_Configuration();


	while (1)
	{
		MainTask();

		if (SysTask.nTick)
		{
			SysTask.nTick--;
			SysTickTask();
		}
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SysTickTask(void)
{
	vu16 static 	u16SecTick = 0; 				//秒计数

	if (u16SecTick++ >= 1000)
	{
		u16SecTick			= 0;

		if (SysTask.nShowTime)
		{
			SysTask.nShowTime--;

			if (SysTask.nShowTime == 0)
				OledInitTask();
		}

	}

	if (SysTask.nLoadTime)
	{
		SysTask.nLoadTime--;

		if (SysTask.nLoadTime == 0 && SysTask.RemoteState == SAN_DEF && SysTask.TouchState == TOUCH_INIT)
		{
			OledInitTask();
		}

	}

	if (SysTask.nWaitTime)
	{
		SysTask.nWaitTime--;
	}

	if (SysTask.MotoAStateTime)
		SysTask.MotoAStateTime--;

	if (SysTask.MotoBStateTime)
		SysTask.MotoBStateTime--;

	if (SysTask.MotoCStateTime)
		SysTask.MotoCStateTime--;

	if (SysTask.MotoDStateTime)
		SysTask.MotoDStateTime--;

	if (SysTask.MotoARunTime)
		SysTask.MotoARunTime--;

	if (SysTask.MotoBRunTime)
		SysTask.MotoBRunTime--;

	if (SysTask.MotoCRunTime)
		SysTask.MotoCRunTime--;

	if (SysTask.MotoDRunTime)
		SysTask.MotoDRunTime--;

	if (SysTask.MotoAWaitTime)
		SysTask.MotoAWaitTime--;

	if (SysTask.MotoBWaitTime)
		SysTask.MotoBWaitTime--;

	if (SysTask.MotoCWaitTime)
		SysTask.MotoCWaitTime--;

	if (SysTask.MotoDWaitTime)
		SysTask.MotoDWaitTime--;


	if (SysTask.ClockAStateTime)
		SysTask.ClockAStateTime--;

	if (SysTask.ClockBStateTime)
		SysTask.ClockBStateTime--;

	if (SysTask.ClockCStateTime)
		SysTask.ClockCStateTime--;

	if (SysTask.ClockDStateTime)
		SysTask.ClockDStateTime--;

	if (SysTask.nFingerSubWaitT)
		SysTask.nFingerSubWaitT--;

	if (SysTask.ClockA_offTime)
	{
		SysTask.ClockA_offTime--;

		if (SysTask.ClockA_offTime == 0)
			GPIO_ResetBits(LOCK_GPIO, LOCK_A_PIN );
	}

	if (SysTask.ClockB_offTime)
	{
		SysTask.ClockB_offTime--;

		if (SysTask.ClockB_offTime == 0)
			GPIO_ResetBits(LOCK_GPIO, LOCK_B_PIN );
	}

	if (SysTask.ClockC_offTime)
	{
		SysTask.ClockC_offTime--;

		if (SysTask.ClockC_offTime == 0)
			GPIO_ResetBits(LOCK_GPIO, LOCK_C_PIN );
	}

	if (SysTask.ClockD_offTime)
	{
		SysTask.ClockD_offTime--;

		if (SysTask.ClockD_offTime == 0)
			GPIO_ResetBits(LOCK_GPIO, LOCK_D_PIN );
	}

	//			if (state)
	//			{
	//				state				= 0;
	//				GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	//			}
	//			else 
	//			{
	//				state				= 1;
	//				GPIO_SetBits(GPIOC, GPIO_Pin_13);
	//			}
}


