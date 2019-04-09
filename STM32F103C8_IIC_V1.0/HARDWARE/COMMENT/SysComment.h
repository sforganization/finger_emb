

#ifndef __SYSCOMMENT_H__
#define __SYSCOMMENT_H__

#ifdef _MAININC_
#define EXTERN

#else

#define EXTERN					extern
#endif


/* ͷ�ļ�  ------------------------------------------------------------*/
#include "stm32f10x.h"



/*
*   �������ڹ�ռ��40k��û���Ż���
*   ��60k��ַ����������洢�� ���ݴ洢λ�ã���Ϊ���ݴ洢��������������д�������������ʱ����flash�Ѿ���
*   ������һ��flash���洢���� 61k  ��62k��63k ���ؿ���3W�� 
*   ���Լ���ָ��ģ��flash 16ҳ 
*/

/* PASSWD [0] [1] [2] [3] [4] [5]   [���ݴ洢��ַƫ��]*/

#define PASSWD_ADDR				0x0800F000		

#define DATA_ADDR				0x0800F400		

#define FLASH_READ_CNT    7
#define DATA_READ_CNT    8

#define LOCK_OFFTIME    4000




#define VOID					void	




#define VOID					void	


//�������������ݴ�С������--���б�������ʹ���ֽ�����
//�豸��Ϣ
#define LED_SHOWFRE_DNS 		300
#define LED_SHOWFRE_OK			1000


#define FINGER_MAX_CNT			10   //ָ��ģ�����洢����

#define LOCK_RCC_CLOCKCMD             RCC_APB2PeriphClockCmd
#define LOCK_RCC_CLOCKGPIO            RCC_APB2Periph_GPIOA

#define LOCK_A_PIN                 GPIO_Pin_8
#define LOCK_B_PIN                 GPIO_Pin_9
#define LOCK_C_PIN                 GPIO_Pin_10
#define LOCK_D_PIN                 GPIO_Pin_11

#define LOCK_GPIO                  GPIOA

#define LOCK_ZERO_RCC_CLOCKCMD             RCC_APB2PeriphClockCmd
#define LOCK_ZERO_RCC_CLOCKGPIO            RCC_APB2Periph_GPIOA

#define LOCK_ZERO_A_PIN                 GPIO_Pin_4
#define LOCK_ZERO_B_PIN                 GPIO_Pin_5
#define LOCK_ZERO_C_PIN                 GPIO_Pin_6
#define LOCK_ZERO_D_PIN                 GPIO_Pin_7

#define LOCK_ZERO_GPIO                  GPIOA




#define PWM_RCC_CLOCKCMD             RCC_APB2PeriphClockCmd
#define PWM_RCC_CLOCKGPIO            RCC_APB2Periph_GPIOB

#define PWM_A_PIN_P                 GPIO_Pin_0  //PWM+
#define PWM_A_PIN_N                 GPIO_Pin_1  //PWM-

#define PWM_B_PIN_P                 GPIO_Pin_10
#define PWM_B_PIN_N                 GPIO_Pin_11

#define PWM_C_PIN_P                 GPIO_Pin_12
#define PWM_C_PIN_N                 GPIO_Pin_13

#define PWM_D_PIN_P                 GPIO_Pin_14
#define PWM_D_PIN_N                 GPIO_Pin_15

#define PWM_GPIO                  GPIOB


#define A_FORM			0x00
#define B_FORM			0x01
#define C_FORM			0x02
#define D_FORM			0x03


#define CLOCK_ON			0x1
#define CLOCK_OFF			0x0




#define REMOTE_KEY_0			0x98
#define REMOTE_KEY_1			0xA2
#define REMOTE_KEY_2			0x62
#define REMOTE_KEY_3			0xE2
#define REMOTE_KEY_4			0x22
#define REMOTE_KEY_5			0x02
#define REMOTE_KEY_6			0xC2
#define REMOTE_KEY_7			0xE0
#define REMOTE_KEY_8			0xA8
#define REMOTE_KEY_9			0x90

#define REMOTE_KEY_MENU 		0x68    // *
#define REMOTE_KEY_RETURN		0xB0	//#
#define REMOTE_KEY_OK			0x38
#define REMOTE_KEY_UP			0x18
#define REMOTE_KEY_DOWN 		0x4A
#define REMOTE_KEY_LEFT 		0x10
#define REMOTE_KEY_RIGHT		0x5A
#define REMOTE_KEY_ERR			0x00  


#define PASSWD_COUNT			0x6   //����λ�� 6λ
#define REMOTE_SHOW_TIME		20   //���յ�ң�� led��ʾʱ�䣬��ʱ���˳�
#define MOTO_ZERO_DETECT		20   //����⣬�ʱ�䣬��ⲻ��ֱ���˳� ,ʵ��ȫ��һȦΪ10S����

//    static u16 u16WriteCount = 0;  //��д����
//    static u16 u16EraseCount = 0;  //�ز�д����
//                FLASH_ReadMoreData(FLASH_SAVE_ADDR, readData, 8);
//                FLASH_WriteMoreData(FLASH_SAVE_ADDR, writeData, 8);
//                FLASH_ReadMoreData(FLASH_SAVE_ADDR, readData, 8);             





typedef enum 
{
    FALSE = 0, TRUE = !FALSE
} bool;


typedef enum 
{
    WRITE_PASSWD = 0, 
    MANAGE_MAIN, //����������
    MANAGE_CHOOSE, //ѡ��������
    MANAGE_MODE, //ģʽ����
    MANAGE_FINGER, //ָ�ƹ������
    MANAGE_PASSWD, //����������
    MANAGE_ADDUSR, //����ָ�ƹ������
    MANAGE_DELUSR, //ɾ��ָ�ƹ������
    MANAGE_CHANGE, //���Ĺ���Ա����
    SAN_DEF = 0XFF, 
} RemoteState_T;


typedef enum 
{
    INIT = 0, 
    ENTER,
    AGAIN,
    WAIT,
    GETKEY,
	SAN_ERR,
} RemoteSub_T;
    
typedef enum 
{
   TOUCH_INIT = 0, 
   TOUCH_CHECK,
   TOUCH_DISPLAY_ID,
   TOUCH_DISPLAY_ID_DEBOU,  //��������
   TOUCH_KEY_CHECK,  //��ⰴ������
   TOUCH_MANAGE_DISPLAY, //�����û�ָ�ƹ���
   TOUCH_MANAGE, //�����û�ָ�ƹ���
   TOUCH_MANAGE_CHOOSE, //�����û�ָ�ƹ���
   TOUCH_ADD_USER, //�����û�ָ�ƹ���
   TOUCH_DEL_USER, //�����û�ָ�ƹ���
   TOUCH_WAIT,
   TOUCH_DEF = 0XFF, 
} TouchState_T;


typedef enum 
{
    TOUCH_SUB_INIT = 0, 
    TOUCH_SUB_TIMER_S,  //ʱ�书��ѡ�� 
    TOUCH_SUB_FR_S,  //����תѡ��
    TOUCH_SUB_CH_TI,  //�ı�ʱ��
    TOUCH_SUB_CH_DI,  //�ı䷽��
    TOUCH_SUB_CH_TI_DE,  //�ı�ʱ��DEBOUNCE ����
    TOUCH_SUB_CH_DI_DE,  //�ı䷽��DEBOUNCE ����
    TOUCH_SUB_ENTER,  //
    TOUCH_SUB_AGAIN,  //
    TOUCH_GETKEY,
    TOUCH_GETKEY_DEBOUNCE, //���ּ��
    TOUCH_SUB_WAIT, 
    TOUCH_SUB_DEF = 0XFF,
} TouchSub_T;    



typedef enum 
{
    MOTO_TIME_TPD = 0, //ÿ��Ķ���ģʽ������12Сʱ��ֹͣ12Сʱ
    MOTO_TIME_650, //��ת2���ӣ�ֹͣ942S
    MOTO_TIME_750,  //��ת2���ӣ�ֹͣ800S
    MOTO_TIME_850,  //��ת2���ӣ�ֹͣ693S
    MOTO_TIME_1000, //��ת2���ӣ�ֹͣ570S
    MOTO_TIME_1950, //��ת2���ӣ�ֹͣ234S

    
    MOTO_TIME_OFF = 0XFE,  //����״̬�£����ֹͣ

    MOTO_TIME_DEF = 0XFF,
}MotoTime;  
    

typedef enum 
{
    MOTO_FR_FWD = 0,  //��ת
    MOTO_FR_REV, //��ת
    MOTO_FR_FWD_REV, //����ת
    MOTO_FR_STOP, //ֹͣ
    MOTO_FR_DEF = 0XFF,
}MotoFR;  

typedef enum 
{
    MOTO_STATE_INIT = 0,  //
    MOTO_STATE_CHANGE_TIME,  //
    MOTO_STATE_CHANGE_DIR,  //
    MOTO_STATE_RUN_NOR,  // ��ת���߷�ת����״̬
    MOTO_STATE_RUN_CHA,  // ����ת����״̬
    MOTO_STATE_STOP,
    MOTO_STATE_WAIT,  //
    MOTO_STATE_IDLE,  //
    MOTO_STATE_DEF = 0XFF,
}MotoState;  

typedef enum 
{
    MOTO_SUB_STATE_RUN = 0,  //
    MOTO_SUB_STATE_WAIT,  //
    MOTO_SUB_STATE_DEF = 0XFF,
}MotoSubState;  

typedef enum 
{
    CLOCK_STATE_DETECT = 0,  //
    CLOCK_STATE_DEBOUNSE,  //����
    CLOCK_STATE_DEF = 0XFF,
}ClockTask_T;  
    

typedef struct 
{
    u16             u16FlashPasswd[FLASH_READ_CNT];  //�������������flash��
    u16             u16FlashData[DATA_READ_CNT];  //��������

    bool			mUpdate;

    vu16			nTick;								//������
    vu16			nLoadTime;								//��ʾ����ʱ��


    vu8 			nShowTime;							//oled �л���ʾʱ�� ��λS
    vu32 			nWaitTime;							//
    vu32 			nSubWaitTime;						//��״̬�ȴ���ʱ
    vu32 			nFingerSubWaitT;						//��״̬�ȴ���ʱ

    vu32 			MotoAWaitTime;						//A���ֹͣʱ��
    vu32 			MotoARunTime;						//A�������ʱ��
    vu32 			MotoBWaitTime;						//���ֹͣʱ��
    vu32 			MotoBRunTime;						//�������ʱ��
    vu32 			MotoCWaitTime;						//���ֹͣʱ��
    vu32 			MotoCRunTime;						//�������ʱ��
    vu32 			MotoDWaitTime;						//���ֹͣʱ��
    vu32 			MotoDRunTime;						//�������ʱ��


    vu32 			MotoAStateTime;						//A״̬����ʱ
    vu32 			MotoBStateTime;						//״̬����ʱ
    vu32 			MotoCStateTime;						//״̬����ʱ
    vu32 			MotoDStateTime;						//״̬����ʱ


    vu32 			ClockAStateTime;						//A״̬����ʱ
    vu32 			ClockBStateTime;						//״̬����ʱ
    vu32 			ClockCStateTime;						//״̬����ʱ
    vu32 			ClockDStateTime;						//״̬����ʱ


    vu32 			ClockA_offTime;						//���ر�ʱ��-1
    vu32            ClockB_offTime;                     //���ر�ʱ��-2
    vu32            ClockC_offTime;                     //���ر�ʱ��-3
    vu32            ClockD_offTime;                     //���ر�ʱ��-4

    RemoteState_T	RemoteState;
    RemoteSub_T	    RemoteSub;
    
    TouchState_T	TouchState;
    TouchSub_T	    TouchSub;


    ClockTask_T    ClockATask;
    ClockTask_T    ClockBTask;
    ClockTask_T    ClockCTask;
    ClockTask_T    ClockDTask;

        
    u8 MotoChoose; //���ѡ��
    

    MotoFR MotoAMode; //���1ģʽ
    MotoFR MotoBMode; //���2ģʽ
    MotoFR MotoCMode; //���3ģʽ
    MotoFR MotoDMode; //���4ģʽ

    MotoFR MotoAModeSave; //���1ģʽ
    MotoFR MotoBModeSave; //���2ģʽ
    MotoFR MotoCModeSave; //���3ģʽ
    MotoFR MotoDModeSave; //���4ģʽ

    MotoTime MotoATime; //���1����ʱ��
    MotoTime MotoBTime; //���2����ʱ��
    MotoTime MotoCTime; //���3����ʱ��
    MotoTime MotoDTime; //���4����ʱ��

    MotoTime MotoATimeSave; //���1����ʱ��
    MotoTime MotoBTimeSave; //���2����ʱ��
    MotoTime MotoCTimeSave; //���3����ʱ��
    MotoTime MotoDTimeSave; //���4����ʱ��

     
    MotoState MotoAState; //���1����״̬
    MotoState MotoBState; //���2����״̬
    MotoState MotoCState; //���3����״̬
    MotoState MotoDState; //���4����״̬


    MotoSubState MotoASubState; //���1����״̬
    MotoSubState MotoBSubState; //���2����״̬
    MotoSubState MotoCSubState; //���3����״̬
    MotoSubState MotoDSubState; //���4����״̬

    
    
    u8 AClockState; // A��״̬ �� or ��
    u8 BClockState; // B��״̬ �� or ��
    u8 CClockState; // C��״̬ �� or ��
    u8 DClockState; // D��״̬ �� or ��
    
    u8 AClockStateSave; // A��״̬ �� or ��
    u8 BClockStateSave; // B��״̬ �� or ��
    u8 CClockStateSave; // C��״̬ �� or ��
    u8 DClockStateSave; // D��״̬ �� or ��


} SYS_TASK;


/* ȫ�ֱ��� -----------------------------------------------------------*/

/* ȫ�ֱ��� -----------------------------------------------------------*/
EXTERN vu8		mSysIWDGDog; //�����
EXTERN vu32 	mSysSoftDog; //�������� 
EXTERN vu16 	mSysTick; //������
EXTERN vu16 	mSysSec; //������
EXTERN vu16 	mTimeRFRX; //���ռ��-����

EXTERN SYS_TASK SysTask;


EXTERN void Sys_DelayMS(uint16_t nms);
EXTERN void Sys_GetMac(u8 * mac);
EXTERN void Sys_LayerInit(void);
EXTERN void Sys_IWDGConfig(u16 time);
EXTERN void Sys_IWDGReloadCounter(void);
EXTERN void Sys_1s_Tick(void);

EXTERN void DelayUs(uint16_t nCount);
EXTERN void DelayMs(uint16_t nCount);
EXTERN void Strcpy(u8 * str1, u8 * str2, u8 len);
EXTERN bool Strcmp(u8 * str1, u8 * str2, u8 len);

EXTERN void MainTask(void);
EXTERN void SysInit(void);
EXTERN void OledInitTask(void);
EXTERN void ShowLaunch(void);


#endif

