

#ifndef __SYSCOMMENT_H__
#define __SYSCOMMENT_H__

#ifdef _MAININC_
#define EXTERN

#else

#define EXTERN					extern
#endif


/* 头文件  ------------------------------------------------------------*/
#include "stm32f10x.h"



/*
*   程序现在共占用40k，没有优化，
*   用60k地址部分做密码存储及 数据存储位置，因为数据存储经常擦除，所以写入与读出不符合时表明flash已经坏
*   跳到另一个flash区存储数据 61k  。62k。63k 保守可以3W次 
*   可以加入指纹模块flash 16页 
*/

/* PASSWD [0] [1] [2] [3] [4] [5]   [数据存储地址偏移]*/

#define PASSWD_ADDR				0x0800F000		

#define DATA_ADDR				0x0800F400		

#define FLASH_READ_CNT    7
#define DATA_READ_CNT    8

#define LOCK_OFFTIME    4000




#define VOID					void	




#define VOID					void	


//联合体由于数据大小端问题--所有变量必须使用字节声明
//设备信息
#define LED_SHOWFRE_DNS 		300
#define LED_SHOWFRE_OK			1000


#define FINGER_MAX_CNT			10   //指纹模块最多存储个数

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


#define PASSWD_COUNT			0x6   //密码位数 6位
#define REMOTE_SHOW_TIME		20   //接收到遥控 led显示时间，超时则退出
#define MOTO_ZERO_DETECT		20   //零点检测，最长时间，检测不到直接退出 ,实测全速一圈为10S左右

//    static u16 u16WriteCount = 0;  //重写次数
//    static u16 u16EraseCount = 0;  //重擦写次数
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
    MANAGE_MAIN, //管理主界面
    MANAGE_CHOOSE, //选择电机界面
    MANAGE_MODE, //模式界面
    MANAGE_FINGER, //指纹管理界面
    MANAGE_PASSWD, //密码管理界面
    MANAGE_ADDUSR, //增加指纹管理界面
    MANAGE_DELUSR, //删除指纹管理界面
    MANAGE_CHANGE, //更改管理员界面
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
   TOUCH_DISPLAY_ID_DEBOU,  //消抖过后
   TOUCH_KEY_CHECK,  //检测按键输入
   TOUCH_MANAGE_DISPLAY, //超级用户指纹管理
   TOUCH_MANAGE, //超级用户指纹管理
   TOUCH_MANAGE_CHOOSE, //超级用户指纹管理
   TOUCH_ADD_USER, //超级用户指纹管理
   TOUCH_DEL_USER, //超级用户指纹管理
   TOUCH_WAIT,
   TOUCH_DEF = 0XFF, 
} TouchState_T;


typedef enum 
{
    TOUCH_SUB_INIT = 0, 
    TOUCH_SUB_TIMER_S,  //时间功能选择 
    TOUCH_SUB_FR_S,  //正反转选择
    TOUCH_SUB_CH_TI,  //改变时间
    TOUCH_SUB_CH_DI,  //改变方向
    TOUCH_SUB_CH_TI_DE,  //改变时间DEBOUNCE 防抖
    TOUCH_SUB_CH_DI_DE,  //改变方向DEBOUNCE 防抖
    TOUCH_SUB_ENTER,  //
    TOUCH_SUB_AGAIN,  //
    TOUCH_GETKEY,
    TOUCH_GETKEY_DEBOUNCE, //松手检测
    TOUCH_SUB_WAIT, 
    TOUCH_SUB_DEF = 0XFF,
} TouchSub_T;    



typedef enum 
{
    MOTO_TIME_TPD = 0, //每天的动作模式，工作12小时，停止12小时
    MOTO_TIME_650, //旋转2分钟，停止942S
    MOTO_TIME_750,  //旋转2分钟，停止800S
    MOTO_TIME_850,  //旋转2分钟，停止693S
    MOTO_TIME_1000, //旋转2分钟，停止570S
    MOTO_TIME_1950, //旋转2分钟，停止234S

    
    MOTO_TIME_OFF = 0XFE,  //开锁状态下，电机停止

    MOTO_TIME_DEF = 0XFF,
}MotoTime;  
    

typedef enum 
{
    MOTO_FR_FWD = 0,  //正转
    MOTO_FR_REV, //反转
    MOTO_FR_FWD_REV, //正反转
    MOTO_FR_STOP, //停止
    MOTO_FR_DEF = 0XFF,
}MotoFR;  

typedef enum 
{
    MOTO_STATE_INIT = 0,  //
    MOTO_STATE_CHANGE_TIME,  //
    MOTO_STATE_CHANGE_DIR,  //
    MOTO_STATE_RUN_NOR,  // 正转或者反转运行状态
    MOTO_STATE_RUN_CHA,  // 正反转运行状态
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
    CLOCK_STATE_DEBOUNSE,  //防抖
    CLOCK_STATE_DEF = 0XFF,
}ClockTask_T;  
    

typedef struct 
{
    u16             u16FlashPasswd[FLASH_READ_CNT];  //保存密码和数据flash区
    u16             u16FlashData[DATA_READ_CNT];  //保存数据

    bool			mUpdate;

    vu16			nTick;								//节拍器
    vu16			nLoadTime;								//显示加载时间


    vu8 			nShowTime;							//oled 切换显示时间 单位S
    vu32 			nWaitTime;							//
    vu32 			nSubWaitTime;						//子状态等待延时
    vu32 			nFingerSubWaitT;						//子状态等待延时

    vu32 			MotoAWaitTime;						//A电机停止时间
    vu32 			MotoARunTime;						//A电机运行时间
    vu32 			MotoBWaitTime;						//电机停止时间
    vu32 			MotoBRunTime;						//电机运行时间
    vu32 			MotoCWaitTime;						//电机停止时间
    vu32 			MotoCRunTime;						//电机运行时间
    vu32 			MotoDWaitTime;						//电机停止时间
    vu32 			MotoDRunTime;						//电机运行时间


    vu32 			MotoAStateTime;						//A状态机延时
    vu32 			MotoBStateTime;						//状态机延时
    vu32 			MotoCStateTime;						//状态机延时
    vu32 			MotoDStateTime;						//状态机延时


    vu32 			ClockAStateTime;						//A状态机延时
    vu32 			ClockBStateTime;						//状态机延时
    vu32 			ClockCStateTime;						//状态机延时
    vu32 			ClockDStateTime;						//状态机延时


    vu32 			ClockA_offTime;						//锁关必时间-1
    vu32            ClockB_offTime;                     //锁关必时间-2
    vu32            ClockC_offTime;                     //锁关必时间-3
    vu32            ClockD_offTime;                     //锁关必时间-4

    RemoteState_T	RemoteState;
    RemoteSub_T	    RemoteSub;
    
    TouchState_T	TouchState;
    TouchSub_T	    TouchSub;


    ClockTask_T    ClockATask;
    ClockTask_T    ClockBTask;
    ClockTask_T    ClockCTask;
    ClockTask_T    ClockDTask;

        
    u8 MotoChoose; //电机选择
    

    MotoFR MotoAMode; //电机1模式
    MotoFR MotoBMode; //电机2模式
    MotoFR MotoCMode; //电机3模式
    MotoFR MotoDMode; //电机4模式

    MotoFR MotoAModeSave; //电机1模式
    MotoFR MotoBModeSave; //电机2模式
    MotoFR MotoCModeSave; //电机3模式
    MotoFR MotoDModeSave; //电机4模式

    MotoTime MotoATime; //电机1运行时间
    MotoTime MotoBTime; //电机2运行时间
    MotoTime MotoCTime; //电机3运行时间
    MotoTime MotoDTime; //电机4运行时间

    MotoTime MotoATimeSave; //电机1运行时间
    MotoTime MotoBTimeSave; //电机2运行时间
    MotoTime MotoCTimeSave; //电机3运行时间
    MotoTime MotoDTimeSave; //电机4运行时间

     
    MotoState MotoAState; //电机1运行状态
    MotoState MotoBState; //电机2运行状态
    MotoState MotoCState; //电机3运行状态
    MotoState MotoDState; //电机4运行状态


    MotoSubState MotoASubState; //电机1运行状态
    MotoSubState MotoBSubState; //电机2运行状态
    MotoSubState MotoCSubState; //电机3运行状态
    MotoSubState MotoDSubState; //电机4运行状态

    
    
    u8 AClockState; // A锁状态 开 or 关
    u8 BClockState; // B锁状态 开 or 关
    u8 CClockState; // C锁状态 开 or 关
    u8 DClockState; // D锁状态 开 or 关
    
    u8 AClockStateSave; // A锁状态 开 or 关
    u8 BClockStateSave; // B锁状态 开 or 关
    u8 CClockStateSave; // C锁状态 开 or 关
    u8 DClockStateSave; // D锁状态 开 or 关


} SYS_TASK;


/* 全局变量 -----------------------------------------------------------*/

/* 全局变量 -----------------------------------------------------------*/
EXTERN vu8		mSysIWDGDog; //软狗标记
EXTERN vu32 	mSysSoftDog; //软狗计数器 
EXTERN vu16 	mSysTick; //节拍器
EXTERN vu16 	mSysSec; //节拍器
EXTERN vu16 	mTimeRFRX; //接收间隔-仿真

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

