
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
#define usart2_baund			57600//����2�����ʣ�����ָ��ģ�鲨���ʸ��ģ�ע�⣺ָ��ģ��Ĭ��57600��



void SysTickTask(void);


void ledinit(void) //DEBUG ��

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
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void LockGPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	LOCK_RCC_CLOCKCMD(LOCK_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LOCK_A_PIN | LOCK_B_PIN | LOCK_C_PIN | LOCK_D_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LOCK_GPIO, &GPIO_InitStructure);

	GPIO_ResetBits(LOCK_GPIO, LOCK_A_PIN | LOCK_B_PIN | LOCK_C_PIN | LOCK_D_PIN);
}


/*******************************************************************************
* ����: 
* ����: �����io
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Moto_ZeroGPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��(����)KEY1���Ŷ�ӦIO�˿�ʱ�� */
	RCC_APB2PeriphClockCmd(LOCK_ZERO_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LOCK_ZERO_A_PIN | LOCK_ZERO_B_PIN | LOCK_ZERO_C_PIN | LOCK_ZERO_D_PIN;
	;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������

	GPIO_Init(LOCK_ZERO_GPIO, &GPIO_InitStructure);
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void PwmGPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(PWM_RCC_CLOCKGPIO, ENABLE); //ʹ��B�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin =
		 PWM_A_PIN_P | PWM_A_PIN_N | PWM_B_PIN_P | PWM_B_PIN_N | PWM_C_PIN_P | PWM_C_PIN_N | PWM_D_PIN_P | PWM_D_PIN_N;

	//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //�ٶ�50MHz
	GPIO_Init(PWM_GPIO, &GPIO_InitStructure);
	GPIO_ResetBits(PWM_GPIO, 
		PWM_A_PIN_P | PWM_A_PIN_N | PWM_B_PIN_P | PWM_B_PIN_N | PWM_C_PIN_P | PWM_C_PIN_N | PWM_D_PIN_P | PWM_D_PIN_N);
}


int main(void)
{
	delay_init();									//��ʱ������ʼ��	  
	NVIC_Configuration();							//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ� 	LED_Init();				 

	//uart_init(115200);								//��ʼ������1������Ϊ115200������֧��USMART	 ��ӡas608�õ�
	usart2_init(usart2_baund);						//��ʼ������2,������ָ��ģ��ͨѶ

	//PS_StaGPIO_Init();								//��ʼ��FR��״̬����  ʶ��ָ�ư�ѹ����ߵ�ƽ����Ϊ���ù�ϵ��ʼ�������ŵ�KEY_GPIO_Init����
	LockGPIOInit();
	Moto_ZeroGPIOInit();
	PwmGPIOInit();

	//BEEP_GPIO_Init();
	//ledinit();										//Debug ���ʱ���Ƿ���ȷ
	OLED_Init();									//��ʼ��OLED  
	KEY_GPIO_Init();								//����IO��ʼ��Ҫ�������ǰ��
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
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SysTickTask(void)
{
	vu16 static 	u16SecTick = 0; 				//�����

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


