
#include "SysComment.h"
#include "stm32f10x_it.h"
#include "stm32f10x_iwdg.h"
#include "remote.h"
#include "san_flash.h"

#include "as608.h"


#include "key.h"

#include "bmp.h"

#include "oled.h"


void FillMainBox(u8 u8BoxIndex);

vu32			u16MoteTime_a[][3] =
{
	//{
	//	MOTO_TIME_OFF, 0, 0
	//},
	{
		MOTO_TIME_TPD, 43200000, 43200000
	},
	{
		MOTO_TIME_650, 120000, 942000
	},
	{
		MOTO_TIME_750, 120000, 800000
	},
	{
		MOTO_TIME_850, 120000, 693000
	},
	{
		MOTO_TIME_1000, 120000, 570000
	},
	{
		MOTO_TIME_1950, 120000, 234000
	},
};



u16 			Key2Val[10][10] =
{
	{
		(u16)
		REMOTE_KEY_0, 0
	},
	{
		(u16)
		REMOTE_KEY_1, 1
	},
	{
		(u16)
		REMOTE_KEY_2, 2
	},
	{
		(u16)
		REMOTE_KEY_3, 3
	},
	{
		(u16)
		REMOTE_KEY_4, 4
	},
	{
		(u16)
		REMOTE_KEY_5, 5
	},
	{
		(u16)
		REMOTE_KEY_6, 6
	},
	{
		(u16)
		REMOTE_KEY_7, 7
	},
	{
		(u16)
		REMOTE_KEY_8, 8
	},
	{
		(u16)
		REMOTE_KEY_9, 9
	},
};


//内部变量
static vu16 	mDelay;
static u8		u8Password_a[PASSWD_COUNT] =
{
	0
};


static u8		u8PasswordSave_a[PASSWD_COUNT] =
{
	0
};


//内部函数
void SysTickConfig(void);


/*******************************************************************************
* 名称: SysTick_Handler
* 功能: 系统时钟节拍1MS
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SysTick_Handler(void)
{
	static u16		Tick_1S = 0;

	mSysTick++;
	mSysSec++;
	mTimeRFRX++;

	if (mDelay)
		mDelay--;

	if (++Tick_1S >= 1000)
	{
		Tick_1S 			= 0;

		if (mSysIWDGDog)
		{
			IWDG_ReloadCounter();					/*喂STM32内置硬件狗*/

			if ((++mSysSoftDog) > 5) /*软狗system  DOG 2S over*/
			{
				mSysSoftDog 		= 0;
				NVIC_SystemReset();
			}
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
void DelayUs(uint16_t nCount)
{
	u32 			del = nCount * 5;

	//48M 0.32uS
	//24M 0.68uS
	//16M 1.02us
	while (del--)
		;
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void DelayMs(uint16_t nCount)
{
	unsigned int	ti;

	for (; nCount > 0; nCount--)
	{
		for (ti = 0; ti < 4260; ti++)
			; //16M/980-24M/1420 -48M/2840
	}
}


/*******************************************************************************
* 名称: Strcpy()
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
******************************************************************************/
void Strcpy(u8 * str1, u8 * str2, u8 len)
{
	for (; len > 0; len--)
	{
		*str1++ 			= *str2++;
	}
}


/*******************************************************************************
* 名称: Strcmp()
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
******************************************************************************/
bool Strcmp(u8 * str1, u8 * str2, u8 len)
{
	for (; len > 0; len--)
	{
		if (*str1++ != *str2++)
			return FALSE;
	}

	return TRUE;
}


/*******************************************************************************
* 名称: Sys_DelayMS()
* 功能: 系统延迟函数
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Sys_DelayMS(uint16_t nms)
{
	mDelay				= nms + 1;

	while (mDelay != 0x0)
		;
}


/*******************************************************************************
* 名称: Sys_LayerInit
* 功能: 系统初始化
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Sys_LayerInit(void)
{
	SysTickConfig();

	mSysSec 			= 0;
	mSysTick			= 0;
	SysTask.mUpdate 	= TRUE;
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Sys_IWDGConfig(u16 time)
{
	/* 写入0x5555,用于允许狗狗寄存器写入功能 */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* 狗狗时钟分频,40K/64=0.625K()*/
	IWDG_SetPrescaler(IWDG_Prescaler_64);

	/* 喂狗时间 TIME*1.6MS .注意不能大于0xfff*/
	IWDG_SetReload(time);

	/* 喂狗*/
	IWDG_ReloadCounter();

	/* 使能狗狗*/
	IWDG_Enable();
}


/*******************************************************************************
* 名称: Sys_IWDGReloadCounter
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Sys_IWDGReloadCounter(void)
{
	mSysSoftDog 		= 0;						//喂软狗
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SysTickConfig(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	/* Setup SysTick Timer for 1ms interrupts  */
	if (SysTick_Config(SystemCoreClock / 1000))
	{
		/* Capture error */
		while (1)
			;
	}

	/* Configure the SysTick handler priority */
	NVIC_SetPriority(SysTick_IRQn, 0x0);

#if (								SYSINFOR_PRINTF == 1)
	printf("SysTickConfig:Tick=%d/Second\r\n", 1000);
#endif
}



/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
u8 RemoteKey2Val(u16 key)
{
	u8 i;

	for (i = 0; i < 10; i++)
	{
		if (key == Key2Val[i][0])
			return Key2Val[i][1];
	}

	return 0xff;
}



/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
char CheckPassword(void)
{
	if (u8Password_a[0])
		return 0;

	return 1;
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteWritePasswd(void)
{
	u8 key;
	u8 u8Position		= 0;						//显示位置
	static u8 u8PasswdCnt = 0;						//密码位数 6位

	switch (SysTask.RemoteSub)
	{
		case INIT: // *
			SysTask.nShowTime = REMOTE_SHOW_TIME; //5秒时间进入显示
			SysTask.RemoteSub = ENTER;
			u8PasswdCnt = 0;
			u8Position = 0;
			OLED_Clear();
			OLED_ShowString(0, 0, "Please enter password:", 12);
			break;

		case ENTER: // 输入密码
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= 5;			//刷新显示时间

				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_RETURN) //返回键
					{
						OledInitTask();
						return;
					}

					u8Password_a[u8PasswdCnt] = key;
					u8PasswdCnt++;
					u8Position			= u8PasswdCnt * 16;

					if (u8PasswdCnt != PASSWD_COUNT)
					{
						OLED_ShowChar(u8Position, 6, '*', 16);
					}
					else 
					{
						if (!CheckPassword()) //密码正确
						{
							SysTask.RemoteState = MANAGE_MAIN;
							SysTask.RemoteSub	= INIT;
						}
						else //密码错误
						{
							OLED_Clear();
							OLED_ShowString(0, 0, "Password Error! Try agin...", 12);
							SysTask.RemoteSub	= WAIT;
						}
					}
				}
			}

			break;

		case WAIT: // *
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
				SysTask.RemoteSub	= ENTER;
				u8PasswdCnt 		= 0;
				u8Position			= 0;
				OLED_Clear();
				OLED_ShowString(0, 0, "Please enter password:", 12);
			}

			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteTaskPasswd(void)
{
	u8 key;

	if (Remote_Rdy && (SysTask.RemoteState == SAN_DEF))
	{
		key 				= Remote_Process();

		if ((key != REMOTE_KEY_ERR) && (key != REMOTE_KEY_RETURN))
		{
			SysTask.RemoteState = WRITE_PASSWD;
			SysTask.RemoteSub	= INIT;
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
void RemoteMotoChoose(void)
{
	u8 key;
	static u8 u8Choose	= 0;						//选择默认选择 a）高亮显示

	switch (SysTask.RemoteSub)
	{
		case INIT: // *
			SysTask.nShowTime = REMOTE_SHOW_TIME; //5秒时间进入显示
			SysTask.RemoteSub = ENTER;
			u8Choose = 0;
			OLED_Clear();
			OLED_ShowString(0, 0, "-> Set A Mode", 12);
			OLED_ShowString(0, 2, "Set B Mode", 12);
			OLED_ShowString(0, 4, "Set C Mode", 12);
			OLED_ShowString(0, 6, "Set D Mode", 12);
			break;

		case ENTER: //选择
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //刷新显示时间

				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_OK) //确定键
					{
						SysTask.RemoteState = MANAGE_MODE;
						SysTask.RemoteSub	= INIT;
						SysTask.MotoChoose	= u8Choose;
					}
					else if (key == REMOTE_KEY_RETURN) //返回键
					{
						SysTask.RemoteState = MANAGE_MAIN;
						SysTask.RemoteSub	= INIT;
						return;
					}
					else if (key == REMOTE_KEY_DOWN) //向下键
					{
						if (u8Choose != 3)
						{
							u8Choose++;
							OLED_Clear();

							if (u8Choose == 1)
							{
								OLED_ShowString(0, 0, "Set A Mode", 12);
								OLED_ShowString(0, 2, "-> Set B Mode", 12);
								OLED_ShowString(0, 4, "Set C Mode", 12);
								OLED_ShowString(0, 6, "Set D Mode", 12);

							}
							else if (u8Choose == 2)
							{
								OLED_ShowString(0, 0, "Set A Mode", 12);
								OLED_ShowString(0, 2, "Set B Mode", 12);
								OLED_ShowString(0, 4, "-> Set C Mode", 12);
								OLED_ShowString(0, 6, "Set D Mode", 12);

							}
							else if (u8Choose == 3)
							{
								OLED_ShowString(0, 0, "Set A Mode", 12);
								OLED_ShowString(0, 2, "Set B Mode", 12);
								OLED_ShowString(0, 4, "Set C Mode", 12);
								OLED_ShowString(0, 6, "-> Set D Mode", 12);

							}
						}
					}
					else if (key == REMOTE_KEY_UP) //向上键
					{
						if (u8Choose != 0)
						{
							u8Choose--;
							OLED_Clear();

							if (u8Choose == 0)
							{
								OLED_ShowString(0, 0, "-> Set A Mode", 12);
								OLED_ShowString(0, 2, "Set B Mode", 12);
								OLED_ShowString(0, 4, "Set C Mode", 12);
								OLED_ShowString(0, 6, "Set D Mode", 12);

							}
							else if (u8Choose == 1)
							{
								OLED_ShowString(0, 0, "Set A Mode", 12);
								OLED_ShowString(0, 2, "-> Set B Mode", 12);
								OLED_ShowString(0, 4, "Set C Mode", 12);
								OLED_ShowString(0, 6, "Set D Mode", 12);

							}
							else if (u8Choose == 2)
							{
								OLED_ShowString(0, 0, "Set A Mode", 12);
								OLED_ShowString(0, 2, "Set B Mode", 12);
								OLED_ShowString(0, 4, "-> Set C Mode", 12);
								OLED_ShowString(0, 6, "Set D Mode", 12);

							}
						}
					}
				}
			}

			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteMotoMode(void)
{
	u8 key;
	static u16 u8Choose = 0;						//选择默认选择 a）高亮显示

	switch (SysTask.RemoteSub)
	{
		case INIT: // *
			SysTask.nShowTime = REMOTE_SHOW_TIME; //5秒时间进入显示
			SysTask.RemoteSub = ENTER;
			u8Choose = 0;
			OLED_Clear();
			OLED_ShowString(0, 0, "-> FWD", 12);
			OLED_ShowString(0, 3, "REV", 12);
			OLED_ShowString(0, 6, "F-R", 12);
			break;

		case ENTER: //选择
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //刷新显示时间

				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_OK) //确定键
					{
						switch (SysTask.MotoChoose)
						{
							case 0x00: // 电机0
								SysTask.MotoAMode = (MotoFR)
								u8Choose;
								break;

							case 0x01: // 电机1
								SysTask.MotoBMode = (MotoFR)
								u8Choose;
								break;

							case 0x02: // 电机2
								SysTask.MotoCMode = (MotoFR)
								u8Choose;
								break;

							case 0x03: // 电机3
								SysTask.MotoDMode = (MotoFR)
								u8Choose;
								break;

							default:
								break;
						}
					}
					else if (key == REMOTE_KEY_RETURN) //返回键
					{
						SysTask.RemoteState = MANAGE_CHOOSE;
						SysTask.RemoteSub	= INIT;
						return;
					}
					else if (key == REMOTE_KEY_DOWN) //向下键
					{
						if (u8Choose != 2)
						{
							u8Choose++;
							OLED_Clear();

							if (u8Choose == 1)
							{
								OLED_ShowString(0, 0, "FWD", 12);
								OLED_ShowString(0, 3, "-> REV", 12);
								OLED_ShowString(0, 6, "F-R", 12);

							}
							else if (u8Choose == 2)
							{
								OLED_ShowString(0, 0, "FWD", 12);
								OLED_ShowString(0, 3, "REV", 12);
								OLED_ShowString(0, 6, "-> F-R", 12);

							}
						}
					}
					else if (key == REMOTE_KEY_UP) //向上键
					{
						if (u8Choose != 0)
						{
							u8Choose--;
							OLED_Clear();

							if (u8Choose == 0)
							{
								OLED_ShowString(0, 0, "-> FWD", 12);
								OLED_ShowString(0, 3, "REV", 12);
								OLED_ShowString(0, 6, "F-R", 12);

							}
							else if (u8Choose == 1)
							{
								OLED_ShowString(0, 0, "FWD", 12);
								OLED_ShowString(0, 3, "-> REV", 12);
								OLED_ShowString(0, 6, "F-R", 12);

							}
						}
					}
				}
			}

			break;

		default:
			break;
	}
}





/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteFingerManage(void)
{
	u8 key;
	static u8 u8Choose	= 0;						//选择默认选择 a）高亮显示

	switch (SysTask.RemoteSub)
	{
		case INIT: // *
			SysTask.nShowTime = REMOTE_SHOW_TIME; //5秒时间进入显示
			SysTask.RemoteSub = ENTER;
			u8Choose = 0;
			OLED_Clear();
			OLED_ShowString(0, 0, "-> Add User", 12);
			OLED_ShowString(0, 3, "Delete User", 12);
			OLED_ShowString(0, 6, "Change Manager", 12);
			break;

		case ENTER: //选择
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //刷新显示时间

				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_OK) //确定键
					{
						if (u8Choose == 0)
						{
							SysTask.RemoteState = MANAGE_ADDUSR;
							SysTask.RemoteSub	= INIT;
						}
						else if (u8Choose == 1)
						{
							SysTask.RemoteState = MANAGE_DELUSR;
							SysTask.RemoteSub	= INIT;
						}
						else if (u8Choose == 2)
						{
							SysTask.RemoteState = MANAGE_CHANGE;
							SysTask.RemoteSub	= INIT;
						}
					}
					else if (key == REMOTE_KEY_RETURN) //返回键
					{
						SysTask.RemoteState = MANAGE_MAIN;
						SysTask.RemoteSub	= INIT;
					}
					else if (key == REMOTE_KEY_DOWN) //向下键
					{
						if (u8Choose != 2)
						{
							u8Choose++;

							if (u8Choose == 1)
							{
								OLED_Clear();
								OLED_ShowString(0, 0, "Add User", 12);
								OLED_ShowString(0, 3, "-> Delete User", 12);
								OLED_ShowString(0, 6, "Change Manager", 12);
							}
							else 
							{
								OLED_Clear();
								OLED_ShowString(0, 0, "Add User", 12);
								OLED_ShowString(0, 3, "Delete User", 12);
								OLED_ShowString(0, 6, "-> Change Manage", 12);
							}
						}
					}
					else if (key == REMOTE_KEY_UP) //向上键
					{
						if (u8Choose != 0)
						{
							u8Choose--;

							if (u8Choose == 1)
							{
								OLED_Clear();
								OLED_ShowString(0, 0, "Add User", 12);
								OLED_ShowString(0, 3, "-> Delete User", 12);
								OLED_ShowString(0, 6, "Change Manager", 12);
							}
							else 
							{
								OLED_Clear();
								OLED_ShowString(0, 0, "-> Add User", 12);
								OLED_ShowString(0, 3, "Delete User", 12);
								OLED_ShowString(0, 6, "Change Manager", 12);
							}
						}
					}
				}
			}

			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
u8 ChecknewPassword(void)
{
	u8 i;


	for (i = 0; i < PASSWD_COUNT; i++)
	{
		if (u8Password_a[i] != u8PasswordSave_a[i])
			return 1;
	}

	return 0;
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteResetPasswd(void)
{
	u8 key;
	u8 u8Position		= 0;						//显示位置
	static u8 u8PasswdCnt = 0;						//密码位数 6位
	u8 u8Result 		= 0;

	switch (SysTask.RemoteSub)
	{
		case INIT: // *
			SysTask.nShowTime = REMOTE_SHOW_TIME; //5秒时间进入显示
			SysTask.RemoteSub = ENTER;
			u8PasswdCnt = 0;
			u8Position = 0;
			OLED_Clear();
			OLED_ShowString(0, 0, "Input new password:", 12);
			break;

		case ENTER: // 输入密码
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= 5;			//刷新显示时间

				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_RETURN) //返回键
					{
						SysTask.RemoteState = MANAGE_MAIN;
						SysTask.RemoteSub	= INIT;
						return;
					}

					u8Password_a[u8PasswdCnt] = key;
					u8PasswdCnt++;
					u8Position			= u8PasswdCnt * 16;

					if (u8PasswdCnt != PASSWD_COUNT)
					{
						OLED_ShowChar(u8Position, 6, '*', 16);
					}
					else 
					{
						u8PasswdCnt 		= 0;
						u8Position			= 0;
						OLED_Clear();
						OLED_ShowString(0, 0, "Input new password agin", 12);
						SysTask.RemoteSub	= AGAIN;
					}
				}
			}

			break;

		case AGAIN: // 再次输入密码
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= 5;			//刷新显示时间
				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_RETURN) //返回键
					{
						SysTask.RemoteState = MANAGE_MAIN;
						SysTask.RemoteSub	= INIT;
						return;
					}

					u8PasswordSave_a[u8PasswdCnt] = key;
					u8PasswdCnt++;
					u8Position			= u8PasswdCnt * 16;

					if (u8PasswdCnt != PASSWD_COUNT)
					{
						OLED_ShowChar(u8Position, 6, '*', 16);
					}
					else 
					{
						if (!ChecknewPassword()) //密码正确
						{
							u8Result			= 0;
							OLED_Clear();
							OLED_ShowString(0, 3, "Success!", 16);
						}
						else //密码错误
						{
							u8Result			= 1;
							OLED_Clear();
							OLED_ShowString(0, 3, "Passwd not match", 16);
						}

						SysTask.nWaitTime	= 2000; //2S 显示时间
						SysTask.RemoteSub	= WAIT;
					}
				}
			}

			break;

		case WAIT: // *
			if (SysTask.nWaitTime == 0)
			{
				SysTask.RemoteState = MANAGE_MAIN;
				SysTask.RemoteSub	= INIT;
			}
			else if (Remote_Rdy)
			{
				if (u8Result == 0)
					SysTask.RemoteState = MANAGE_MAIN;

				SysTask.RemoteSub	= INIT;

			}

			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteMainManage(void)
{
	u8 key;
	static u8 u8Choose	= 0;						//选择默认选择 a）高亮显示

	switch (SysTask.RemoteSub)
	{
		case INIT: // *
			SysTask.nShowTime = REMOTE_SHOW_TIME; //5秒时间进入显示
			SysTask.RemoteSub = ENTER;
			u8Choose = 0;
			OLED_Clear();
			OLED_ShowString(0, 0, "-> Mode", 12);
			OLED_ShowString(0, 3, "Fingerprint", 12);
			OLED_ShowString(0, 6, "Reset Passwd", 12);
			break;

		case ENTER: //选择
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //刷新显示时间

				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_OK) //确定键
					{
						if (u8Choose == 0)
						{
							SysTask.RemoteState = MANAGE_MODE;
							SysTask.RemoteSub	= INIT;
						}
						else if (u8Choose == 1)
						{
							SysTask.RemoteState = MANAGE_FINGER;
							SysTask.RemoteSub	= INIT;
						}
						else if (u8Choose == 2)
						{
							SysTask.RemoteState = MANAGE_PASSWD;
							SysTask.RemoteSub	= INIT;
						}
					}
					else if (key == REMOTE_KEY_RETURN) //返回键
					{
						OledInitTask();
						return;
					}
					else if (key == REMOTE_KEY_DOWN) //向下键
					{
						if (u8Choose != 2)
						{
							u8Choose++;

							if (u8Choose == 1)
							{
								OLED_Clear();
								OLED_ShowString(0, 0, "Mode", 12);
								OLED_ShowString(0, 3, "-> Fingerprint", 12);
								OLED_ShowString(0, 6, "Reset Passwd", 12);
							}
							else 
							{
								OLED_Clear();
								OLED_ShowString(0, 0, "Mode", 12);
								OLED_ShowString(0, 3, "Fingerprint", 12);
								OLED_ShowString(0, 6, "-> Reset Passwd", 12);
							}
						}
					}
					else if (key == REMOTE_KEY_UP) //向上键
					{
						if (u8Choose != 0)
						{
							u8Choose--;

							if (u8Choose == 1)
							{
								OLED_Clear();
								OLED_ShowString(0, 0, "Mode", 12);
								OLED_ShowString(0, 3, "-> Fingerprint", 12);
								OLED_ShowString(0, 6, "Reset Passwd", 12);
							}
							else 
							{
								OLED_Clear();
								OLED_ShowString(0, 0, "-> Mode", 12);
								OLED_ShowString(0, 3, "Fingerprint", 12);
								OLED_ShowString(0, 6, "Reset Passwd", 12);
							}
						}
					}
				}
			}

			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteAddUser(void)
{
	u8 key;
	u8 ensure;

	//	u8 u8Result 		= 0;
	u16 u8Shownum;
	u16 u16FingerCnt	= 0;						//指纹模版个数
	static u16 u16FingerID = 0; 					//指纹模版ID
	static u8 u8Position = 0;						//显示位置
	static u8 u8InputCnt = 0;						//输入个数

	switch (SysTask.RemoteSub)
	{
		case INIT: // *
			if (SysTask.nWaitTime == 0)
			{
				PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数

				if (u16FingerCnt >= FINGER_MAX_CNT)
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.RemoteSub	= WAIT;
					SysTask.nWaitTime	= 2000; 	//延时一定时间退出
					OLED_Clear();
					OLED_ShowString(0, 3, "Fingerprint template full！", 12);
				}
				else 
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.RemoteSub	= GETKEY;
					u16FingerID 		= 0;
					u8Position			= 0;
					u8InputCnt			= 0;
					OLED_Clear();
					OLED_ShowString(0, 0, "Input ID to store", 12);
					OLED_ShowString(40, 3, "(0 ~ 9)", 12);
				}
			}

			break;

		case ENTER: //选择
			if ((PS_Sta) && (SysTask.nWaitTime == 0)) //如果有指纹按下
			{

				SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
				ensure				= PS_GetImage();

				if (ensure == 0x00)
				{
					ensure				= PS_GenChar(CharBuffer1); //生成特征

					if (ensure == 0x00)
					{
						OLED_Clear();
						OLED_ShowString(0, 3, "Please press  agin", 12);
						SysTask.nWaitTime	= 2000; //延时一定时间再去采集
						SysTask.RemoteSub	= AGAIN; //跳到第二步						
					}
				}
			}

			break;

		case AGAIN:
			if ((PS_Sta) && (SysTask.nWaitTime == 0)) //如果有指纹按下
			{
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
				ensure				= PS_GetImage();

				if (ensure == 0x00)
				{
					ensure				= PS_GenChar(CharBuffer2); //生成特征

					if (ensure == 0x00)
					{
						ensure				= PS_Match(); //对比两次指纹

						if (ensure == 0x00) //成功
						{

							ensure				= PS_RegModel(); //生成指纹模板

							if (ensure == 0x00)
							{

								ensure				= PS_StoreChar(CharBuffer2, u16FingerID); //储存模板

								if (ensure == 0x00)
								{
									OLED_Clear();
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.RemoteSub	= WAIT;
									SysTask.nWaitTime	= 3000; //延时一定时间退出
									OLED_ShowString(0, 3, "Add User Success", 16);
								}
								else 
								{ //失败
									OLED_Clear();
									OLED_ShowString(0, 3, "Store Failed", 16);
								}
							}
							else 
							{ //失败
								OLED_Clear();
								OLED_ShowString(0, 3, "Generate Template  Failed", 12);
							}
						}
						else 
						{ //失败
							OLED_Clear();
							OLED_ShowString(0, 3, "Match  Failed", 12);
						}
					}
				}
			}

			break;

		case GETKEY: // *
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //刷新显示时间

				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_OK) //确定键
					{
						if (u8InputCnt != 0)
						{
							SysTask.RemoteSub	= ENTER;

							SysTask.nWaitTime	= 2000;
							OLED_Clear();
							OLED_ShowString(0, 3, "Please press finger", 12);
						}
						else 
						{
							SysTask.RemoteSub	= INIT;
							SysTask.nWaitTime	= 2000;
							OLED_Clear();
							OLED_ShowString(0, 0, "NOT input ID", 12);
						}
					}
					else if (key == REMOTE_KEY_RETURN) //返回键
					{
						SysTask.RemoteState = MANAGE_FINGER;
						SysTask.RemoteSub	= INIT;
					}
					else if ((key == REMOTE_KEY_0) || (key == REMOTE_KEY_1) || (key == REMOTE_KEY_2) ||
						 (key == REMOTE_KEY_3) || (key == REMOTE_KEY_4) || (key == REMOTE_KEY_5) ||
						 (key == REMOTE_KEY_6) || (key == REMOTE_KEY_7) || (key == REMOTE_KEY_8) ||
						 (key == REMOTE_KEY_9))
					{
						u8Shownum			= RemoteKey2Val(key);
						u16FingerID 		= u16FingerID * 10 + u8Shownum;
						u8Position			= 32 + u8InputCnt * 16;

						OLED_ShowChar(u8Position, 6, '0' + u8Shownum, 16);

						if ((u16FingerID >= FINGER_MAX_CNT) || (u8InputCnt++ >= 3))
						{
							SysTask.RemoteSub	= INIT;
							SysTask.nWaitTime	= 2500;

							OLED_Clear();
							OLED_ShowString(0, 0, "number More than MAX", 12);
							OLED_ShowString(0, 6, "Try agin", 16);
						}

					}
				}
			}

			break;

		case WAIT: // *
			if ((SysTask.nWaitTime == 0) || (Remote_Rdy))
			{
				SysTask.RemoteState = MANAGE_FINGER;
				SysTask.RemoteSub	= INIT;
			}

			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteDelUser(void)
{
	u8 key;
	u8 ensure;

	//	u8 u8Result 		= 0;
	u16 u8Shownum;
	u16 u16FingerCnt	= 0;						//指纹模版个数
	static u16 u16FingerID = 0; 					//指纹模版ID
	static u8 u8Position = 0;						//显示位置
	static u8 u8InputCnt = 0;						//输入个数

	switch (SysTask.RemoteSub)
	{
		case INIT: // *
			if (SysTask.nWaitTime == 0)
			{
				PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数

				if (u16FingerCnt == 0)
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.RemoteSub	= WAIT;
					SysTask.nWaitTime	= 2000; 	//延时一定时间退出
					OLED_Clear();
					OLED_ShowString(0, 3, "Fingerprint template empty！", 12);
				}
				else 
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.RemoteSub	= GETKEY;
					u16FingerID 		= 0;
					u8Position			= 0;
					u8InputCnt			= 0;
					OLED_Clear();
					OLED_ShowString(0, 0, "Input ID to del", 12);
					OLED_ShowString(30, 3, "(0 ~ 9)", 12);
				}
			}

			break;

		case ENTER: //选择
			if (u16FingerID == 300) //全部删除
				ensure = PS_Empty();
			else 
				ensure = PS_DeletChar(u16FingerID, 1);

			if (ensure == 0)
			{

				OLED_Clear();
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
				SysTask.RemoteSub	= WAIT;
				SysTask.nWaitTime	= 3000; 		//延时一定时间退出
				OLED_ShowString(0, 3, "Del User Success", 16);

			}
			else 
			{
				OLED_Clear();
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
				SysTask.RemoteSub	= INIT;
				SysTask.nWaitTime	= 3000; 		//延时一定时间退出
				OLED_ShowString(0, 3, "Del User Failed", 16);

			}

			//PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数
			break;

		case GETKEY: // *
			if (Remote_Rdy)
			{
				SysTask.nShowTime	= REMOTE_SHOW_TIME; //刷新显示时间

				key 				= Remote_Process();

				if (key != REMOTE_KEY_ERR)
				{
					if (key == REMOTE_KEY_OK) //确定键
					{
						if (u8InputCnt != 0)
						{
							SysTask.RemoteSub	= ENTER;

							SysTask.nWaitTime	= 2000;
							OLED_Clear();
						}
						else 
						{
							SysTask.RemoteSub	= INIT;
							SysTask.nWaitTime	= 2000;
							OLED_Clear();
							OLED_ShowString(0, 0, "NOT input ID", 12);
						}
					}
					else if (key == REMOTE_KEY_RETURN) //返回键
					{
						SysTask.RemoteState = MANAGE_FINGER;
						SysTask.RemoteSub	= INIT;
					}
					else if ((key == REMOTE_KEY_0) || (key == REMOTE_KEY_1) || (key == REMOTE_KEY_2) ||
						 (key == REMOTE_KEY_3) || (key == REMOTE_KEY_4) || (key == REMOTE_KEY_5) ||
						 (key == REMOTE_KEY_6) || (key == REMOTE_KEY_7) || (key == REMOTE_KEY_8) ||
						 (key == REMOTE_KEY_9))
					{
						u8Shownum			= RemoteKey2Val(key);
						u16FingerID 		= u16FingerID * 10 + u8Shownum;
						u8Position			= 32 + u8InputCnt * 16;

						OLED_ShowChar(u8Position, 6, '0' + u8Shownum, 16);


						if ((u16FingerID >= FINGER_MAX_CNT) || (u8InputCnt++ >= 3))
						{
							SysTask.RemoteSub	= INIT;
							SysTask.nWaitTime	= 2500;

							OLED_Clear();
							OLED_ShowString(0, 0, "number More than MAX", 12);
							OLED_ShowString(0, 6, "Try agin", 16);
						}

					}
				}
			}

			break;

		case WAIT: // *
			if ((SysTask.nWaitTime == 0) || (Remote_Rdy))
			{
				SysTask.RemoteState = MANAGE_FINGER;
				SysTask.RemoteSub	= INIT;
			}

			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void RemoteTask(void)
{
	switch (SysTask.RemoteState)
	{
		case WRITE_PASSWD:
			RemoteWritePasswd();
			break;

		case MANAGE_MAIN:
			RemoteMainManage();
			break;

		case MANAGE_CHOOSE:
			RemoteMotoChoose();
			break;

		case MANAGE_MODE:
			RemoteMotoMode();
			break;

		case MANAGE_FINGER:
			RemoteFingerManage();
			break;

		case MANAGE_PASSWD:
			RemoteResetPasswd();
			break;

		case MANAGE_ADDUSR:
			RemoteAddUser();
			break;

		case MANAGE_DELUSR:
			RemoteDelUser();
			break;

		case MANAGE_CHANGE:
			//RemoteChangeManager();
			break;

		default:
			RemoteTaskPasswd();
			break;
	}
}


#define MATCH_FINGER_CNT		4

void ShowLockMode(void)
{
	OLED_Clear();
	OLED_DrawBMP(0, 0, 128, 8, BMP_Main);			//显示主界面

	if (SysTask.MotoAState == MOTO_STATE_IDLE)
		//llMainBox(A_FORM);
    {
        OLED_DrawBMP(0, 0, 32, 4, BMP_fill);
        } 

	if (SysTask.MotoBState == MOTO_STATE_IDLE)
		//FillMainBox(B_FORM);
		OLED_DrawBMP(32, 0, 64, 4, BMP_fill);

	if (SysTask.MotoCState == MOTO_STATE_IDLE)
		//FillMainBox(C_FORM);
    OLED_DrawBMP(64, 0, 96, 4, BMP_fill);

	if (SysTask.MotoDState == MOTO_STATE_IDLE)
		//FillMainBox(D_FORM);
		OLED_DrawBMP(96, 0, 128, 4, BMP_fill);
}


#define DEBOUNCE_TIME			40 //40ms 防抖
void FingerTouchTask(void)
{
	SearchResult seach;
	u8 ensure;
	static u8 u8MatchCnt = 0;						//匹配失败次数，默认匹配MATCH_FINGER_CNT次 
	static u8 u8MotoSelect = 0;
	static u8 u8SelectPosi = 0;
	static u16 u16FingerID = 0; 					//指纹模版ID
	u16 u16FingerCnt	= 0;						//指纹模版个数


	if ((SysTask.RemoteState == SAN_DEF) || (SysTask.RemoteState == WRITE_PASSWD))
	{
		switch (SysTask.TouchState)
		{
			case TOUCH_INIT:
				if (PS_Sta) //有指纹按下
				{
					SysTask.nWaitTime	= 100;		//一段时间再检测
					SysTask.TouchState	= TOUCH_CHECK;
					u8MatchCnt			= 0;
				}

				break;

			case TOUCH_CHECK:
				if (SysTask.nWaitTime == 0) //有指纹按下
				{
					ensure				= PS_GetImage();

					if (ensure == 0x00) //生成特征成功
					{
						//PS_ValidTempleteNum(&u16FingerID); //读库指纹个数
						ensure				= PS_GenChar(CharBuffer1);

						if (ensure == 0x00) //生成特征成功
						{
							ensure				= PS_Search(CharBuffer1, 0, FINGER_MAX_CNT, &seach);

							if (ensure == 0) //匹配成功
							{
								u16FingerID 		= seach.pageID;
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.nWaitTime	= SysTask.nTick + 1000; //延时一定时间 实测此时只会延时300ms
								SysTask.TouchSub	= TOUCH_SUB_INIT;

								if (u16FingerID == 0) //超级用户管理
								{
									SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
								}
								else 
								{
									SysTask.TouchState	= TOUCH_DISPLAY_ID;
								}

								OLED_Clear();
								OLED_ShowString(32, 0, "User ID ", 16);

								OLED_ShowChar(40, 3, '0' + u16FingerID / 100, 16);
								OLED_ShowChar(56, 3, '0' + (u16FingerID / 10) % 10, 16);
								OLED_ShowChar(72, 3, '0' + (u16FingerID) % 10, 16);


								//								OLED_DrawBMP(0, 4, 48, 8, BMP_clock);
								//								OLED_DrawBMP(80, 4, 128, 8, BMP_FwdRev);
							}
							else if (u8MatchCnt >= MATCH_FINGER_CNT)
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.nWaitTime	= SysTask.nTick + 2000; //延时一定时间退出
								SysTask.TouchState	= TOUCH_WAIT;


								OLED_Clear();
								OLED_ShowString(0, 3, "Can't fined fingerprin", 16);
							}
						}
					}

					if (ensure) //匹配失败
					{
						u8MatchCnt++;
					}
				}

				break;

			case TOUCH_MANAGE_DISPLAY:
				if ((SysTask.nWaitTime == 0) && (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) != 0) //按时间选择键 松手检测，不然一直闪
				&& (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) != 0) //按HOME键
				&& (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //确定键
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.nWaitTime	= 20;		//2s显示ID时间
					SysTask.TouchState	= TOUCH_MANAGE;
					SysTask.TouchSub	= TOUCH_SUB_INIT;
					u8SelectPosi		= 0;

					OLED_Clear();
					OLED_ShowString(48, 0, "Manage", 12);
					OLED_ShowString(32, 3, "-> Add User", 12);
					OLED_ShowString(32, 5, "Del User", 12);
				}

				break;

			case TOUCH_MANAGE:
				if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.TouchState	= TOUCH_MANAGE_CHOOSE; //防抖,松手检测
				}

				if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
				{
                           SysTask.nShowTime = 0;
                
                               OledInitTask();
				}

				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.TouchSub	= TOUCH_SUB_INIT;

					if (u8SelectPosi == 0)
					{
						SysTask.TouchState	= TOUCH_ADD_USER;
					}
					else 
					{
						SysTask.TouchState	= TOUCH_DEL_USER;
					}

				}

				break;

			case TOUCH_MANAGE_CHOOSE: //防抖,松手检测
				if (SysTask.nFingerSubWaitT != 0)
				{
					break;
				}

				if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
				|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
				|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //确定键
				{
					SysTask.nFingerSubWaitT = 20;	//20ms检测一次
				}
				else 
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示

					if (++u8SelectPosi >= 2)
						u8SelectPosi = 0;

					switch (u8SelectPosi)
					{
						case 0:
							OLED_Clear();
							OLED_ShowString(48, 0, "Manage", 12);
							OLED_ShowString(32, 3, "-> Add User", 12);
							OLED_ShowString(32, 5, "Del User", 12);
							break;

						case 1:
							OLED_Clear();
							OLED_ShowString(48, 0, "Manage", 12);
							OLED_ShowString(32, 3, "Add User", 12);
							OLED_ShowString(32, 5, "-> Del User", 12);
							break;

						default:
							u8SelectPosi = 0;
							break;
					}

					SysTask.TouchState	= TOUCH_MANAGE;
				}

				break;

			case TOUCH_ADD_USER:
				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键松手检测
				{
					break;
				}

				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT: // *
							if (SysTask.nWaitTime == 0)
							{
								PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数

								if (u16FingerCnt >= FINGER_MAX_CNT)
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.nWaitTime	= 2000; //延时一定时间退出
									SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
									OLED_Clear();
									OLED_ShowString(0, 3, "Fingerprint template full！", 12);
								}
								else 
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.TouchSub	= TOUCH_GETKEY;
									u16FingerID 		= 0;
									u8SelectPosi		= 0;
									OLED_Clear();
									OLED_ShowString(0, 0, "Choose ID (1~9)", 12);
									OLED_ShowString(32, 2, "-> 1", 12);
									OLED_ShowString(32, 4, "2", 12);
									OLED_ShowString(32, 6, "3", 12);
								}
							}

							break;

						case TOUCH_GETKEY: // *
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示

								SysTask.TouchSub	= TOUCH_GETKEY_DEBOUNCE; //防抖,松手检测
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_ENTER;
								u16FingerID 		= u8SelectPosi;
								SysTask.nWaitTime	= 1000;
								OLED_Clear();
								OLED_ShowString(0, 3, "Please press", 12);
							}

							break;

						case TOUCH_GETKEY_DEBOUNCE: //防抖,松手检测
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0)) //按时间选择键
							{
								SysTask.nFingerSubWaitT = 20; //20ms检测一次
							}
							else 
							{
								if (++u8SelectPosi >= 10)
									u8SelectPosi = 1;

								if (u8SelectPosi == 8)
								{
									OLED_ShowString(32, 2, "-> 8", 12);
									OLED_ShowString(32, 4, "9", 12);
									OLED_ShowString(32, 6, "1", 12);

								}
								else if (u8SelectPosi == 9)
								{
									OLED_ShowString(32, 2, "-> 9", 12);
									OLED_ShowString(32, 4, "1", 12);
									OLED_ShowString(32, 6, "2", 12);
								}
								else 
								{
									OLED_ShowString(32, 2, "-> ", 12);
									OLED_ShowChar(56, 2, '0' + u8SelectPosi, 12);
									OLED_ShowChar(32, 4, '1' + u8SelectPosi, 12);
									OLED_ShowChar(32, 6, '2' + u8SelectPosi, 12);
								}

								SysTask.TouchSub	= TOUCH_GETKEY;

							}

							break;

						case TOUCH_SUB_ENTER: //选择
							if ((PS_Sta) && (SysTask.nWaitTime == 0)) //如果有指纹按下
							{

								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								ensure				= PS_GetImage();

								if (ensure == 0x00)
								{
									ensure				= PS_GenChar(CharBuffer1); //生成特征

									if (ensure == 0x00)
									{
										OLED_Clear();
										OLED_ShowString(0, 3, "Please press  agin", 12);
										SysTask.nWaitTime	= 1000; //延时一定时间再去采集
										SysTask.TouchSub	= TOUCH_SUB_AGAIN; //跳到第二步						
									}
								}
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						case TOUCH_SUB_AGAIN:
							if ((PS_Sta) && (SysTask.nWaitTime == 0)) //如果有指纹按下
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								ensure				= PS_GetImage();

								if (ensure == 0x00)
								{
									ensure				= PS_GenChar(CharBuffer2); //生成特征

									if (ensure == 0x00)
									{
										ensure				= PS_Match(); //对比两次指纹

										if (ensure == 0x00) //成功
										{

											ensure				= PS_RegModel(); //生成指纹模板

											if (ensure == 0x00)
											{

												ensure				= PS_StoreChar(CharBuffer2, u16FingerID); //储存模板

												if (ensure == 0x00)
												{
													OLED_Clear();
													SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
													SysTask.TouchSub	= TOUCH_SUB_WAIT;
													SysTask.nWaitTime	= 3000; //延时一定时间退出
													OLED_ShowString(0, 3, "Add User Success", 16);
												}
												else 
												{ //失败
													OLED_Clear();
													OLED_ShowString(0, 3, "Store Failed", 16);
													SysTask.TouchSub	= TOUCH_SUB_ENTER;
													SysTask.nWaitTime	= 3000; //延时一定时间退出
												}
											}
											else 
											{ //失败
												OLED_Clear();
												OLED_ShowString(0, 3, "Generate Template  Failed", 12);
												SysTask.TouchSub	= TOUCH_SUB_ENTER;
												SysTask.nWaitTime	= 3000; //延时一定时间退出
											}
										}
										else 
										{ //失败
											OLED_Clear();
											OLED_ShowString(0, 3, "Match  Failed", 12);
											SysTask.TouchSub	= TOUCH_SUB_ENTER;
											SysTask.nWaitTime	= 3000; //延时一定时间退出
										}
									}
								}
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						case TOUCH_SUB_WAIT: // *
							if ((SysTask.nWaitTime == 0))
							{
								SysTask.TouchState	= TOUCH_MANAGE;
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						default:
							break;
					}
				}
				break;

			case TOUCH_DEL_USER:
				if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键松手检测
				{
					break;
				}

				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT: // *
							if (SysTask.nWaitTime == 0)
							{
								PS_ValidTempleteNum(&u16FingerCnt); //读库指纹个数

								if (u16FingerCnt == 0)
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.nWaitTime	= 2000; //延时一定时间退出
									SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
									OLED_Clear();
									OLED_ShowString(0, 3, "Fingerprint template Null！", 12);
								}
								else 
								{
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.TouchSub	= TOUCH_GETKEY;
									u16FingerID 		= 0;
									u8SelectPosi		= 0;
									OLED_Clear();
									OLED_ShowString(0, 0, "Del ID (1~9)", 12);
									OLED_ShowString(32, 2, "-> 1", 12);
									OLED_ShowString(32, 4, "2", 12);
									OLED_ShowString(32, 6, "3", 12);
								}
							}

							break;

						case TOUCH_GETKEY: // *
							if (SysTask.nWaitTime)
							{
								break;
							}

							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示

								SysTask.TouchSub	= TOUCH_GETKEY_DEBOUNCE; //防抖,松手检测
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT;
								u16FingerID 		= u8SelectPosi;
								SysTask.nWaitTime	= 1000;
								OLED_Clear();
								OLED_ShowString(0, 3, "Sure Del ?", 12);
								OLED_ShowString(0, 5, "ID : ", 12);
								OLED_ShowChar(48, 5, '0' + u16FingerID, 12);
							}

							break;

						case TOUCH_GETKEY_DEBOUNCE: //防抖,松手检测
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0)) //按时间选择键
							{
								SysTask.nFingerSubWaitT = 20; //20ms检测一次
							}
							else 
							{
								if (++u8SelectPosi >= 10)
									u8SelectPosi = 1;

								if (u8SelectPosi == 8)
								{
									OLED_ShowString(32, 2, "-> 8", 12);
									OLED_ShowString(32, 4, "9", 12);
									OLED_ShowString(32, 6, "1", 12);

								}
								else if (u8SelectPosi == 9)
								{
									OLED_ShowString(32, 2, "-> 9", 12);
									OLED_ShowString(32, 4, "1", 12);
									OLED_ShowString(32, 6, "2", 12);
								}
								else 
								{
									OLED_ShowString(32, 2, "-> ", 12);
									OLED_ShowChar(56, 2, '0' + u8SelectPosi, 12);
									OLED_ShowChar(32, 4, '1' + u8SelectPosi, 12);
									OLED_ShowChar(32, 6, '2' + u8SelectPosi, 12);
								}

								SysTask.TouchSub	= TOUCH_GETKEY;

							}

							break;

						case TOUCH_SUB_ENTER: //选择
							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_GETKEY;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_AGAIN;
							}

							break;

						case TOUCH_SUB_AGAIN:
							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0) //确定键松手检测
							{
								ensure				= PS_DeletChar(u16FingerID, 1);

								if (ensure == 0)
								{

									OLED_Clear();
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.TouchSub	= TOUCH_GETKEY;
									SysTask.nWaitTime	= 1000; //延时一定时间退出
									OLED_ShowString(0, 3, "Del User Success", 16);

								}
								else 
								{
									OLED_Clear();
									SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
									SysTask.TouchSub	= TOUCH_GETKEY;
									SysTask.nWaitTime	= 1000; //延时一定时间退出
									OLED_ShowString(0, 3, "Del User Failed", 16);

								}
							}

							break;

						case TOUCH_SUB_WAIT: // *
							if ((GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //确定键松手检测
							{
								SysTask.TouchSub	= TOUCH_SUB_ENTER;
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_MANAGE_DISPLAY;
							}

							break;

						default:
							break;
					}
				}
				break;

			case TOUCH_DISPLAY_ID:
				if ((SysTask.nWaitTime == 0) && (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) != 0) //按时间选择键 松手检测，不然一直闪
				&& (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) != 0) //按HOME键
				&& (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) != 0)) //确定键
				{
					SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
					SysTask.nWaitTime	= 20;		//2s显示ID时间
					SysTask.TouchState	= TOUCH_KEY_CHECK;
					SysTask.TouchSub	= TOUCH_SUB_INIT;

					ShowLockMode();

				}

				break;

			case TOUCH_KEY_CHECK:
				if (SysTask.nWaitTime == 0)
				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT:
                            if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
			
			SysTask.nShowTime = 0;

				OledInitTask();
							}
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //防抖动
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								//SysTask.AClockState = ~(SysTask.AClockState) & 0x01;//取反
								if ((SysTask.MotoAState == MOTO_STATE_STOP) ||
									 (SysTask.MotoAState == MOTO_STATE_IDLE))
								{
									SysTask.MotoAState	= MOTO_STATE_INIT;
									ShowLockMode();
                                    SysTask.AClockState = CLOCK_OFF; //关锁状态

								}
								else 
								{
									SysTask.AClockState = CLOCK_ON; //开锁状态
								}

							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //防抖动
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoBState == MOTO_STATE_STOP) ||
									 (SysTask.MotoBState == MOTO_STATE_IDLE))
								{
									SysTask.MotoBState	= MOTO_STATE_INIT;
									ShowLockMode();
									SysTask.BClockState = CLOCK_OFF; //关锁状态

								}
								else 
								{
									SysTask.BClockState = CLOCK_ON; //开锁状态
								}
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //防抖动
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoCState == MOTO_STATE_STOP) ||
									 (SysTask.MotoCState == MOTO_STATE_IDLE))
								{
									SysTask.MotoCState	= MOTO_STATE_INIT;
									ShowLockMode();
                                    SysTask.CClockState = CLOCK_OFF; //关锁状态

								}
								else 
								{
									SysTask.CClockState = CLOCK_ON; //开锁状态
								}
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //按键按下 
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_WAIT; //防抖动
								SysTask.nWaitTime	= DEBOUNCE_TIME;

								if ((SysTask.MotoDState == MOTO_STATE_STOP) ||
									 (SysTask.MotoDState == MOTO_STATE_IDLE))
								{
									SysTask.MotoDState	= MOTO_STATE_INIT;
									ShowLockMode();
                                    SysTask.DClockState = CLOCK_OFF; //关锁状态

								}
								else 
								{
									SysTask.DClockState = CLOCK_ON; //开锁状态
								}
							}

							/* *******************************************************************
							*	 PAUSE		 home		  RATION
							*	 时间选择		 返回 			正反转选择  
							*
							*
							**/
							//功能键按下
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_TIMER_S;
								u8MotoSelect		= 0;

								//								u8FunSelect 		= 0;
								OLED_Clear();
								OLED_ShowString(0, 0, "Choose which one", 12);
								OLED_DrawBMP(0, 3, 128, 7, BMP_FourClock); //显示主界面

							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //正反转选择
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_FR_S;
								u8MotoSelect		= 0;

								//								u8FunSelect 		= 0;
								OLED_Clear();
								OLED_ShowString(0, 0, "Choose which one", 12);
								OLED_DrawBMP(0, 3, 128, 7, BMP_FourClock); //显示主界面
							}

							break;

						case TOUCH_SUB_TIMER_S: //时间功能选择
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //按键按下
							{
								u8MotoSelect		= 1;
							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //按键按下
							{
								u8MotoSelect		= 2;
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //按键按下
							{
								u8MotoSelect		= 3;
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //按键按下 
							{
								u8MotoSelect		= 4;
							}

							if (u8MotoSelect != 0) //有按键按下
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_TI;
								SysTask.nFingerSubWaitT = 0;
								u8SelectPosi		= 0;
								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								OLED_ShowString(48, 2, "->TPD", 12);
								OLED_ShowString(48, 3, "650", 12);
								OLED_ShowString(48, 4, "750", 12);
								OLED_ShowString(48, 5, "850", 12);
								OLED_ShowString(48, 6, "1000", 12);
								OLED_ShowString(48, 7, "1950", 12);
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							break;

						case TOUCH_SUB_FR_S: //正反转选择
							if (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								u8MotoSelect		= 1;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								u8MotoSelect		= 2;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) == 0) //按键按下
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								u8MotoSelect		= 3;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) == 0) //按键按下 
							{
								SysTask.nShowTime	= MOTO_ZERO_DETECT; //5秒时间进入显示
								u8MotoSelect		= 4;
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							if (u8MotoSelect != 0) //有按键按下
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.nFingerSubWaitT = 0;
								u8SelectPosi		= 0;
								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								OLED_ShowString(48, 1, "->FWD", 12);
								OLED_ShowString(48, 3, "REV", 12);
								OLED_ShowString(48, 5, "F-R", 12);
							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							break;

						case TOUCH_SUB_CH_TI: //改变时间
							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_TI_DE;

								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								if (++u8SelectPosi >= 6)
									u8SelectPosi = 0;

								switch (u8SelectPosi)
								{
									case 0:
										OLED_ShowString(48, 2, "->TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 1:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "->650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 2:
										OLED_ShowString(48, 1, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "->750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 3:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "->850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 4:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "->1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 5:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "->1950", 12);
										break;

									default:
										u8SelectPosi = 0;
										break;
								}


							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_TI_DE;
                                switch (u8SelectPosi)
								{
									case 0:
										OLED_ShowString(48, 2, "->TPD *", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 1:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "->650 *", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 2:
										OLED_ShowString(48, 1, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "->750 *", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 3:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "->850 *", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 4:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "->1000 *", 12);
										OLED_ShowString(48, 7, "1950", 12);
										break;

									case 5:
										OLED_ShowString(48, 2, "TPD", 12);
										OLED_ShowString(48, 3, "650", 12);
										OLED_ShowString(48, 4, "750", 12);
										OLED_ShowString(48, 5, "850", 12);
										OLED_ShowString(48, 6, "1000", 12);
										OLED_ShowString(48, 7, "->1950 *", 12);
										break;

									default:
										u8SelectPosi = 0;
										break;
								}

								switch (u8MotoSelect) //ABCD
								{
									case 1:
										SysTask.MotoATime = (MotoTime)
										u8SelectPosi;
										SysTask.MotoARunTime = u16MoteTime_a[u8SelectPosi + 1][1];
										SysTask.MotoAWaitTime = u16MoteTime_a[u8SelectPosi + 1][2];
										break;

									case 2:
										SysTask.MotoBTime = (MotoTime)
										u8SelectPosi;
										SysTask.MotoBRunTime = u16MoteTime_a[u8SelectPosi + 1][1];
										SysTask.MotoBWaitTime = u16MoteTime_a[u8SelectPosi + 1][2];
										break;

									case 3:
										SysTask.MotoCTime = (MotoTime)
										u8SelectPosi;
										SysTask.MotoCRunTime = u16MoteTime_a[u8SelectPosi + 1][1];
										SysTask.MotoCWaitTime = u16MoteTime_a[u8SelectPosi + 1][2];
										break;

									case 4:
										SysTask.MotoDTime = (MotoTime)
										u8SelectPosi;
										SysTask.MotoDRunTime = u16MoteTime_a[u8SelectPosi + 1][1];
										SysTask.MotoDWaitTime = u16MoteTime_a[u8SelectPosi + 1][2];
										break;

									default:
										break;
								}
							}

							break;

						case TOUCH_SUB_CH_TI_DE: //改变时间，防抖,松手检测
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //确定键
							{
								SysTask.nFingerSubWaitT = 20; //20ms检测一次
							}
							else 
							{
								SysTask.TouchSub	= TOUCH_SUB_CH_TI;
							}

							break;

						case TOUCH_SUB_CH_DI: //改变方向
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_DI_DE;

								if (++u8SelectPosi >= 3)
									u8SelectPosi = 0;

								OLED_Clear();
								OLED_ShowChar(0, 0, 'A' + u8MotoSelect - 1, 16);
								OLED_DrawHorizontal(0, 16, 2, 4, 1);

								switch (u8SelectPosi)
								{
									case 0:
										OLED_ShowString(48, 1, "->FWD", 12);
										OLED_ShowString(48, 3, "REV", 12);
										OLED_ShowString(48, 5, "F-R", 12);
										break;

									case 1:
										OLED_ShowString(48, 1, "FWD", 12);
										OLED_ShowString(48, 3, "->REV", 12);
										OLED_ShowString(48, 5, "F-R", 12);
										break;

									case 2:
										OLED_ShowString(48, 1, "FWD", 12);
										OLED_ShowString(48, 3, "REV", 12);
										OLED_ShowString(48, 5, "->F-R", 12);
										break;

									default:
										u8SelectPosi = 0;
										break;
								}


							}

							if (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							{
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchState	= TOUCH_DISPLAY_ID;
							}

							if (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0) //确定键
							{
								switch (u8SelectPosi)
								{
									case 0:
										OLED_ShowString(48, 1, "->FWD *", 12);
										OLED_ShowString(48, 3, "REV", 12);
										OLED_ShowString(48, 5, "F-R", 12);
										break;

									case 1:
										OLED_ShowString(48, 1, "FWD", 12);
										OLED_ShowString(48, 3, "->REV *", 12);
										OLED_ShowString(48, 5, "F-R", 12);
										break;

									case 2:
										OLED_ShowString(48, 1, "FWD", 12);
										OLED_ShowString(48, 3, "REV", 12);
										OLED_ShowString(48, 5, "->F-R *", 12);
										break;

									default:
										u8SelectPosi = 0;
										break;
								}
								SysTask.nShowTime	= REMOTE_SHOW_TIME; //5秒时间进入显示
								SysTask.TouchSub	= TOUCH_SUB_CH_DI_DE;

								switch (u8MotoSelect)
								{
									case 1:
										SysTask.MotoAMode = (MotoFR)
										u8SelectPosi;
										break;

									case 2:
										SysTask.MotoBMode = (MotoFR)
										u8SelectPosi;
										;
										break;

									case 3:
										SysTask.MotoCMode = (MotoFR)
										u8SelectPosi;
										;
										break;

									case 4:
										SysTask.MotoDMode = (MotoFR)
										u8SelectPosi;
										;
										break;

									default:
										break;
								}
							}

							break;

						case TOUCH_SUB_CH_DI_DE: //改变方向 debounce
							if (SysTask.nFingerSubWaitT != 0)
							{
								break;
							}

							if ((GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_GPIO_PIN) == 0) //按时间选择键
							|| (GPIO_ReadInputDataBit(KEY6_GPIO, KEY6_GPIO_PIN) == 0) //按HOME键
							|| (GPIO_ReadInputDataBit(KEY7_GPIO, KEY7_GPIO_PIN) == 0)) //确定键
							{
								SysTask.nFingerSubWaitT = 20; //20ms检测一次
							}
							else 
							{
								SysTask.TouchSub	= TOUCH_SUB_CH_DI;
							}

							break;

						case TOUCH_SUB_WAIT: //防抖
							if (SysTask.nWaitTime != 0)
								break;

							if ((GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_GPIO_PIN) != 0) &&
								 (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_GPIO_PIN) != 0))
							{
								SysTask.TouchSub	= TOUCH_SUB_INIT;
								SysTask.nWaitTime	= DEBOUNCE_TIME; //防止一开始就检测到0点
							}

							break;

						default:
							break;
					}
				}

				break;

			case TOUCH_WAIT:
				if (SysTask.nWaitTime == 0)
				{
					OledInitTask();
				}
				else if (Remote_Rdy)
				{
					SysTask.nWaitTime	= 0;
				}

				break;

			default:
				break;
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
void Moto_A_Task(void)
{
	static u8 u8State	= 0;

	//A电机
	switch (SysTask.MotoAState)
	{
		case MOTO_STATE_INIT:
			if (SysTask.MotoAStateTime == 0)
			{
				SysTask.MotoARunTime = u16MoteTime_a[SysTask.MotoATime][1];
				SysTask.MotoAState	= MOTO_STATE_CHANGE_DIR;
				SysTask.MotoASubState = MOTO_SUB_STATE_RUN;
			}

			break;

		case MOTO_STATE_CHANGE_TIME:
			break;

		case MOTO_STATE_CHANGE_DIR:
			if (SysTask.MotoAMode == MOTO_FR_FWD)
			{
				GPIO_SetBits(PWM_GPIO, PWM_A_PIN_P); //正转
				GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_N); //
				SysTask.MotoAState	= MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.MotoAMode == MOTO_FR_REV)
			{
				GPIO_SetBits(PWM_GPIO, PWM_A_PIN_N); //反转
				GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_P); //
				SysTask.MotoAState	= MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.MotoAMode == MOTO_FR_FWD_REV)
			{ //正反转
				u8State 			= 0;
				GPIO_SetBits(PWM_GPIO, PWM_A_PIN_P); //先正转
				GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_N); //
				SysTask.MotoAState	= MOTO_STATE_RUN_CHA;
			}

			SysTask.MotoAModeSave = SysTask.MotoAMode;
			break;

		case MOTO_STATE_RUN_NOR: //普通运行状态
			if (SysTask.MotoAMode != SysTask.MotoAModeSave)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_P | PWM_A_PIN_N); //先暂停电机转动
				SysTask.MotoAState	= MOTO_STATE_INIT; //先延时一段时间再翻转
				SysTask.MotoAStateTime = 200;
				break;
			}

			switch (SysTask.MotoASubState)
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.MotoARunTime == 0)
					{
						SysTask.MotoAWaitTime = u16MoteTime_a[SysTask.MotoATime][2];
						SysTask.MotoASubState = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_P); //停止
						GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_N); //
					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.MotoAWaitTime == 0)
					{
						SysTask.MotoARunTime = u16MoteTime_a[SysTask.MotoATime][1];
						SysTask.MotoASubState = MOTO_SUB_STATE_RUN;

						if (SysTask.MotoAMode == MOTO_FR_FWD)
						{
							GPIO_SetBits(PWM_GPIO, PWM_A_PIN_P); //正转
							GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_N); //
							SysTask.MotoAState	= MOTO_STATE_RUN_NOR;
						}
						else if (SysTask.MotoAMode == MOTO_FR_REV)
						{
							GPIO_SetBits(PWM_GPIO, PWM_A_PIN_N); //反转
							GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_P); //
							SysTask.MotoAState	= MOTO_STATE_RUN_NOR;
						}
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_RUN_CHA: //正反转运行状态
			if (SysTask.MotoAMode != SysTask.MotoAModeSave)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_P | PWM_A_PIN_N); //先暂停电机转动
				SysTask.MotoAState	= MOTO_STATE_INIT; //先延时一段时间再翻转
				break;
			}

			switch (SysTask.MotoASubState)
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.MotoARunTime == 0)
					{
						SysTask.MotoAWaitTime = u16MoteTime_a[SysTask.MotoATime][2];
						SysTask.MotoASubState = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_P); //停止
						GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_N); //
					}
					else if (SysTask.MotoAStateTime == 0)
					{
						if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_A_PIN) == 0) // //检测到零点  
						{

							SysTask.MotoAStateTime = SysTask.nTick + 500; //检测到0点，一段时间后再次检测，避开

							if (u8State == 0)
							{
								u8State 			= 1;
								GPIO_SetBits(PWM_GPIO, PWM_A_PIN_N); //反转
								GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_P); //

							}
							else 
							{
								u8State 			= 0;
								GPIO_SetBits(PWM_GPIO, PWM_A_PIN_P); //先正转
								GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_N); //
							}
						}

					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.MotoAWaitTime == 0)
					{
						u8State 			= 0;
						SysTask.MotoARunTime = u16MoteTime_a[SysTask.MotoATime][1];
						SysTask.MotoASubState = MOTO_SUB_STATE_RUN;
						GPIO_SetBits(PWM_GPIO, PWM_A_PIN_P); //先正转
						GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_N); //
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_STOP:
			if (SysTask.MotoAWaitTime == 0)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_N); //停止
				GPIO_ResetBits(PWM_GPIO, PWM_A_PIN_P); //

				SysTask.MotoAState	= MOTO_STATE_IDLE;
			}

			break;

		case MOTO_STATE_WAIT:
			break;

		case MOTO_STATE_IDLE:
			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Moto_B_Task(void)
{
	static u8 u8State	= 0;

	//B电机
	switch (SysTask.MotoBState)
	{
		case MOTO_STATE_INIT:
			if (SysTask.MotoBStateTime == 0)
			{
				SysTask.MotoBRunTime = u16MoteTime_a[SysTask.MotoBTime][1];
				SysTask.MotoBState	= MOTO_STATE_CHANGE_DIR;
				SysTask.MotoBSubState = MOTO_SUB_STATE_RUN;
			}

			break;

		case MOTO_STATE_CHANGE_TIME:
			break;

		case MOTO_STATE_CHANGE_DIR:
			if (SysTask.MotoBMode == MOTO_FR_FWD)
			{
				GPIO_SetBits(PWM_GPIO, PWM_B_PIN_P); //正转
				GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_N); //
				SysTask.MotoBState	= MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.MotoBMode == MOTO_FR_REV)
			{
				GPIO_SetBits(PWM_GPIO, PWM_B_PIN_N); //反转
				GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_P); //
				SysTask.MotoBState	= MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.MotoBMode == MOTO_FR_FWD_REV)
			{ //正反转
				u8State 			= 0;
				GPIO_SetBits(PWM_GPIO, PWM_B_PIN_P); //先正转
				GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_N); //
				SysTask.MotoBState	= MOTO_STATE_RUN_CHA;
			}

			SysTask.MotoBModeSave = SysTask.MotoBMode;
			break;

		case MOTO_STATE_RUN_NOR: //普通运行状态
			if (SysTask.MotoBMode != SysTask.MotoBModeSave)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_P | PWM_B_PIN_N); //先暂停电机转动
				SysTask.MotoBState	= MOTO_STATE_INIT; //先延时一段时间再翻转
				SysTask.MotoBStateTime = 200;
				break;
			}

			switch (SysTask.MotoBSubState)
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.MotoBRunTime == 0)
					{
						SysTask.MotoBWaitTime = u16MoteTime_a[SysTask.MotoBTime][2];
						SysTask.MotoBSubState = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_P); //停止
						GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_N); //
					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.MotoBWaitTime == 0)
					{
						SysTask.MotoBRunTime = u16MoteTime_a[SysTask.MotoBTime][1];
						SysTask.MotoBSubState = MOTO_SUB_STATE_RUN;

						if (SysTask.MotoBMode == MOTO_FR_FWD)
						{
							GPIO_SetBits(PWM_GPIO, PWM_B_PIN_P); //正转
							GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_N); //
							SysTask.MotoBState	= MOTO_STATE_RUN_NOR;
						}
						else if (SysTask.MotoBMode == MOTO_FR_REV)
						{
							GPIO_SetBits(PWM_GPIO, PWM_B_PIN_N); //反转
							GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_P); //
							SysTask.MotoBState	= MOTO_STATE_RUN_NOR;
						}
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_RUN_CHA: //正反转运行状态
			if (SysTask.MotoBMode != SysTask.MotoBModeSave)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_P | PWM_B_PIN_N); //先暂停电机转动
				SysTask.MotoBState	= MOTO_STATE_INIT; //先延时一段时间再翻转
				break;
			}

			switch (SysTask.MotoBSubState)
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.MotoBRunTime == 0)
					{
						SysTask.MotoBWaitTime = u16MoteTime_a[SysTask.MotoBTime][2];
						SysTask.MotoBSubState = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_P); //停止
						GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_N); //
					}
					else if (SysTask.MotoBStateTime == 0)
					{
						if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_B_PIN) == 0) // //检测到零点  
						{

							SysTask.MotoBStateTime = SysTask.nTick + 500; //检测到0点，一段时间后再次检测，避开

							if (u8State == 0)
							{
								u8State 			= 1;
								GPIO_SetBits(PWM_GPIO, PWM_B_PIN_N); //反转
								GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_P); //

							}
							else 
							{
								u8State 			= 0;
								GPIO_SetBits(PWM_GPIO, PWM_B_PIN_P); //先正转
								GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_N); //
							}
						}

					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.MotoBWaitTime == 0)
					{
						u8State 			= 0;
						SysTask.MotoBRunTime = u16MoteTime_a[SysTask.MotoBTime][1];
						SysTask.MotoBSubState = MOTO_SUB_STATE_RUN;
						GPIO_SetBits(PWM_GPIO, PWM_B_PIN_P); //先正转
						GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_N); //
					}

					break;

				default:
					break;
			}


		case MOTO_STATE_STOP:
			if (SysTask.MotoBWaitTime == 0)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_N); //停止
				GPIO_ResetBits(PWM_GPIO, PWM_B_PIN_P);
				SysTask.MotoBState	= MOTO_STATE_IDLE;
			}

			break;

		case MOTO_STATE_IDLE:
			break;

		case MOTO_STATE_WAIT:
			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Moto_C_Task(void)
{
	static u8 u8State	= 0;

	//A电机
	switch (SysTask.MotoCState)
	{
		case MOTO_STATE_INIT:
			if (SysTask.MotoCStateTime == 0)
			{
				SysTask.MotoCRunTime = u16MoteTime_a[SysTask.MotoCTime][1];
				SysTask.MotoCState	= MOTO_STATE_CHANGE_DIR;
				SysTask.MotoCSubState = MOTO_SUB_STATE_RUN;
			}

			break;

		case MOTO_STATE_CHANGE_TIME:
			break;

		case MOTO_STATE_CHANGE_DIR:
			if (SysTask.MotoCMode == MOTO_FR_FWD)
			{
				GPIO_SetBits(PWM_GPIO, PWM_C_PIN_P); //正转
				GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_N); //
				SysTask.MotoCState	= MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.MotoCMode == MOTO_FR_REV)
			{
				GPIO_SetBits(PWM_GPIO, PWM_C_PIN_N); //反转
				GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_P); //
				SysTask.MotoCState	= MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.MotoCMode == MOTO_FR_FWD_REV)
			{ //正反转
				u8State 			= 0;
				GPIO_SetBits(PWM_GPIO, PWM_C_PIN_P); //先正转
				GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_N); //
				SysTask.MotoCState	= MOTO_STATE_RUN_CHA;
			}

			SysTask.MotoCModeSave = SysTask.MotoCMode;
			break;

		case MOTO_STATE_RUN_NOR: //普通运行状态
			if (SysTask.MotoCMode != SysTask.MotoCModeSave)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_P | PWM_C_PIN_N); //先暂停电机转动
				SysTask.MotoCState	= MOTO_STATE_INIT; //先延时一段时间再翻转
				SysTask.MotoCStateTime = 200;
				break;
			}

			switch (SysTask.MotoCSubState)
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.MotoCRunTime == 0)
					{
						SysTask.MotoCWaitTime = u16MoteTime_a[SysTask.MotoCTime][2];
						SysTask.MotoCSubState = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_P); //停止
						GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_N); //
					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.MotoCWaitTime == 0)
					{
						SysTask.MotoCRunTime = u16MoteTime_a[SysTask.MotoCTime][1];
						SysTask.MotoCSubState = MOTO_SUB_STATE_RUN;

						if (SysTask.MotoCMode == MOTO_FR_FWD)
						{
							GPIO_SetBits(PWM_GPIO, PWM_C_PIN_P); //正转
							GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_N); //
							SysTask.MotoCState	= MOTO_STATE_RUN_NOR;
						}
						else if (SysTask.MotoCMode == MOTO_FR_REV)
						{
							GPIO_SetBits(PWM_GPIO, PWM_C_PIN_N); //反转
							GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_P); //
							SysTask.MotoCState	= MOTO_STATE_RUN_NOR;
						}
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_RUN_CHA: //正反转运行状态
			if (SysTask.MotoCMode != SysTask.MotoCModeSave)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_P | PWM_C_PIN_N); //先暂停电机转动
				SysTask.MotoCState	= MOTO_STATE_INIT; //先延时一段时间再翻转
				break;
			}

			switch (SysTask.MotoCSubState)
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.MotoCRunTime == 0)
					{
						SysTask.MotoCWaitTime = u16MoteTime_a[SysTask.MotoCTime][2];
						SysTask.MotoCSubState = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_P); //停止
						GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_N); //
					}
					else if (SysTask.MotoCStateTime == 0)
					{
						if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_C_PIN) == 0) // //检测到零点  
						{

							SysTask.MotoCStateTime = SysTask.nTick + 500; //检测到0点，一段时间后再次检测，避开

							if (u8State == 0)
							{
								u8State 			= 1;
								GPIO_SetBits(PWM_GPIO, PWM_C_PIN_N); //反转
								GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_P); //

							}
							else 
							{
								u8State 			= 0;
								GPIO_SetBits(PWM_GPIO, PWM_C_PIN_P); //先正转
								GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_N); //
							}
						}

					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.MotoCWaitTime == 0)
					{
						u8State 			= 0;
						SysTask.MotoCRunTime = u16MoteTime_a[SysTask.MotoCTime][1];
						SysTask.MotoCSubState = MOTO_SUB_STATE_RUN;
						GPIO_SetBits(PWM_GPIO, PWM_C_PIN_P); //先正转
						GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_N); //
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_STOP:
			if (SysTask.MotoCWaitTime == 0)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_N); //停止
				GPIO_ResetBits(PWM_GPIO, PWM_C_PIN_P); //
				SysTask.MotoCState	= MOTO_STATE_IDLE;
			}

			break;

		case MOTO_STATE_IDLE:
			break;

		case MOTO_STATE_WAIT:
			break;

		default:
			break;
	}
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void Moto_D_Task(void)
{
	static u8 u8State	= 0;

	//D电机
	switch (SysTask.MotoDState)
	{
		case MOTO_STATE_INIT:
			if (SysTask.MotoDStateTime == 0)
			{
				SysTask.MotoDRunTime = u16MoteTime_a[SysTask.MotoDTime][1];
				SysTask.MotoDState	= MOTO_STATE_CHANGE_DIR;
				SysTask.MotoDSubState = MOTO_SUB_STATE_RUN;
			}

			break;

		case MOTO_STATE_CHANGE_TIME:
			break;

		case MOTO_STATE_CHANGE_DIR:
			if (SysTask.MotoDMode == MOTO_FR_FWD)
			{
				GPIO_SetBits(PWM_GPIO, PWM_D_PIN_P); //正转
				GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_N); //
				SysTask.MotoDState	= MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.MotoDMode == MOTO_FR_REV)
			{
				GPIO_SetBits(PWM_GPIO, PWM_D_PIN_N); //反转
				GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_P); //
				SysTask.MotoDState	= MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.MotoDMode == MOTO_FR_FWD_REV)
			{ //正反转
				u8State 			= 0;
				GPIO_SetBits(PWM_GPIO, PWM_D_PIN_P); //先正转
				GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_N); //
				SysTask.MotoDState	= MOTO_STATE_RUN_CHA;
			}

			SysTask.MotoDModeSave = SysTask.MotoDMode;
			break;

		case MOTO_STATE_RUN_NOR: //普通运行状态
			if (SysTask.MotoDMode != SysTask.MotoDModeSave)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_P | PWM_D_PIN_N); //先暂停电机转动
				SysTask.MotoDState	= MOTO_STATE_INIT; //先延时一段时间再翻转
				SysTask.MotoDStateTime = 200;
				break;
			}

			switch (SysTask.MotoDSubState)
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.MotoDRunTime == 0)
					{
						SysTask.MotoDWaitTime = u16MoteTime_a[SysTask.MotoDTime][2];
						SysTask.MotoDSubState = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_P); //停止
						GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_N); //
					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.MotoDWaitTime == 0)
					{
						SysTask.MotoDRunTime = u16MoteTime_a[SysTask.MotoDTime][1];
						SysTask.MotoDSubState = MOTO_SUB_STATE_RUN;

						if (SysTask.MotoDMode == MOTO_FR_FWD)
						{
							GPIO_SetBits(PWM_GPIO, PWM_D_PIN_P); //正转
							GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_N); //
							SysTask.MotoDState	= MOTO_STATE_RUN_NOR;
						}
						else if (SysTask.MotoDMode == MOTO_FR_REV)
						{
							GPIO_SetBits(PWM_GPIO, PWM_D_PIN_N); //反转
							GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_P); //
							SysTask.MotoDState	= MOTO_STATE_RUN_NOR;
						}
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_RUN_CHA: //正反转运行状态
			if (SysTask.MotoDMode != SysTask.MotoDModeSave)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_P | PWM_D_PIN_N); //先暂停电机转动
				SysTask.MotoDState	= MOTO_STATE_INIT; //先延时一段时间再翻转
				break;
			}

			switch (SysTask.MotoDSubState)
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.MotoDRunTime == 0)
					{
						SysTask.MotoDWaitTime = u16MoteTime_a[SysTask.MotoDTime][2];
						SysTask.MotoDSubState = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_P); //停止
						GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_N); //
					}
					else if (SysTask.MotoDStateTime == 0)
					{
						if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_D_PIN) == 0) // //检测到零点  
						{

							SysTask.MotoDStateTime = SysTask.nTick + 500; //检测到0点，一段时间后再次检测，避开

							if (u8State == 0)
							{
								u8State 			= 1;
								GPIO_SetBits(PWM_GPIO, PWM_D_PIN_N); //反转
								GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_P); //

							}
							else 
							{
								u8State 			= 0;
								GPIO_SetBits(PWM_GPIO, PWM_D_PIN_P); //先正转
								GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_N); //
							}
						}

					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.MotoDWaitTime == 0)
					{
						u8State 			= 0;
						SysTask.MotoDRunTime = u16MoteTime_a[SysTask.MotoDTime][1];
						SysTask.MotoDSubState = MOTO_SUB_STATE_RUN;
						GPIO_SetBits(PWM_GPIO, PWM_D_PIN_P); //先正转
						GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_N); //
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_STOP:
			if (SysTask.MotoDWaitTime == 0)
			{
				GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_N); //停止
				GPIO_ResetBits(PWM_GPIO, PWM_D_PIN_P); //
				SysTask.MotoDState	= MOTO_STATE_IDLE;
			}

			break;

		case MOTO_STATE_IDLE:
			break;

		case MOTO_STATE_WAIT:
			break;

		default:
			break;
	}
}


#define MOTO_DEBOUNCE			40	//40ms 延时到达中点位置

/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void ClockTask(void)
{

	switch (SysTask.ClockATask) //A锁
	{
		case CLOCK_STATE_DETECT:
			if (SysTask.AClockState != SysTask.AClockStateSave)
			{
				if (SysTask.AClockState == CLOCK_ON)
				{
					if (SysTask.MotoAWaitTime != 0) //如果电机在停止状态
					{
						SysTask.MotoAWaitTime = 0;
					}

					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_A_PIN) == 0) // //检测到零点 开锁
					{
						SysTask.ClockAStateTime = MOTO_DEBOUNCE; //延时40ms到达中点
						SysTask.MotoAWaitTime = MOTO_DEBOUNCE;
						SysTask.ClockATask	= CLOCK_STATE_DEBOUNSE;
                        //FillMainBox(A_FORM);
						OLED_DrawBMP(0, 0, 32, 4, BMP_fill);
						SysTask.MotoAState	= MOTO_STATE_STOP; //停止转动
						SysTask.AClockStateSave = SysTask.AClockState;
                        SysTask.ClockA_offTime= LOCK_OFFTIME;
					}
				}
				else 
				{
					SysTask.AClockStateSave = SysTask.AClockState;
					GPIO_ResetBits(LOCK_GPIO, LOCK_A_PIN);
                    SysTask.ClockA_offTime = 0;
				}
			}

			break;

		case CLOCK_STATE_DEBOUNSE:
			if (SysTask.ClockAStateTime == 0)
			{
				if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_A_PIN) == 0) // //检测到零点 开锁
				{
					GPIO_SetBits(LOCK_GPIO, LOCK_A_PIN);
					SysTask.ClockATask	= CLOCK_STATE_DETECT;
				}
			}

			break;

		default:
			break;
	}

	switch (SysTask.ClockBTask) //B锁
	{
		case CLOCK_STATE_DETECT:
			if (SysTask.BClockState != SysTask.BClockStateSave)
			{
				if (SysTask.BClockState == CLOCK_ON)
				{
					if (SysTask.MotoBWaitTime != 0) //如果电机在停止状态
					{
						SysTask.MotoBWaitTime = 0;
					}

					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_B_PIN) == 0) // //检测到零点 开锁
					{
						SysTask.ClockBStateTime = MOTO_DEBOUNCE; //延时40ms到达中点
						SysTask.MotoBWaitTime = MOTO_DEBOUNCE;
						SysTask.ClockBTask	= CLOCK_STATE_DEBOUNSE;
						//FillMainBox(B_FORM);
						OLED_DrawBMP(32, 0, 64, 4, BMP_fill);
						SysTask.MotoBState	= MOTO_STATE_STOP; //停止转动
						SysTask.BClockStateSave = SysTask.BClockState;
                        SysTask.ClockB_offTime= LOCK_OFFTIME;
					}
				}
				else 
				{
					SysTask.BClockStateSave = SysTask.BClockState;
					GPIO_ResetBits(LOCK_GPIO, LOCK_A_PIN);
                    SysTask.ClockB_offTime = 0;
				}
			}

			break;

		case CLOCK_STATE_DEBOUNSE:
			if (SysTask.ClockBStateTime == 0)
			{
				if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_B_PIN) == 0) // //检测到零点 开锁
				{
					GPIO_SetBits(LOCK_GPIO, LOCK_B_PIN);
					SysTask.ClockBTask	= CLOCK_STATE_DETECT;
				}
			}

			break;

		default:
			break;
	}

	switch (SysTask.ClockCTask) //C锁
	{
		case CLOCK_STATE_DETECT:
			if (SysTask.CClockState != SysTask.CClockStateSave)
			{
				if (SysTask.CClockState == CLOCK_ON)
				{
					if (SysTask.MotoAWaitTime != 0) //如果电机在停止状态
					{
						SysTask.MotoAWaitTime = 0;
					}

					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_C_PIN) == 0) // //检测到零点 开锁
					{
						SysTask.ClockCStateTime = MOTO_DEBOUNCE; //延时40ms到达中点
						SysTask.MotoCWaitTime = MOTO_DEBOUNCE;
						SysTask.ClockCTask	= CLOCK_STATE_DEBOUNSE;
						//FillMainBox(C_FORM);
						OLED_DrawBMP(64, 0, 96, 4, BMP_fill);
						SysTask.MotoCState	= MOTO_STATE_STOP; //停止转动
						SysTask.CClockStateSave = SysTask.CClockState;
                        SysTask.ClockC_offTime= LOCK_OFFTIME;
					}
				}
				else 
				{
					SysTask.CClockStateSave = SysTask.CClockState;
					GPIO_ResetBits(LOCK_GPIO, LOCK_C_PIN);
                    SysTask.ClockC_offTime = 0;
				}
			}

			break;

		case CLOCK_STATE_DEBOUNSE:
			if (SysTask.ClockCStateTime == 0)
			{
				if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_C_PIN) == 0) // //检测到零点 开锁
				{
					GPIO_SetBits(LOCK_GPIO, LOCK_C_PIN);
					SysTask.ClockCTask	= CLOCK_STATE_DETECT;
				}
			}

			break;

		default:
			break;
	}

	switch (SysTask.ClockDTask) //D锁
	{
		case CLOCK_STATE_DETECT:
			if (SysTask.DClockState != SysTask.DClockStateSave)
			{
				if (SysTask.DClockState == CLOCK_ON)
				{
					if (SysTask.MotoDWaitTime != 0) //如果电机在停止状态
					{
						SysTask.MotoDWaitTime = 0;
					}

					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_D_PIN) == 0) // //检测到零点 开锁
					{
						SysTask.ClockDStateTime = MOTO_DEBOUNCE; //延时40ms到达中点
						SysTask.MotoDWaitTime = MOTO_DEBOUNCE;
						SysTask.ClockDTask	= CLOCK_STATE_DEBOUNSE;
						//FillMainBox(D_FORM);
						OLED_DrawBMP(96, 0, 128, 4, BMP_fill);
						SysTask.MotoDState	= MOTO_STATE_STOP; //停止转动
						SysTask.DClockStateSave = SysTask.DClockState;
                        SysTask.ClockD_offTime= LOCK_OFFTIME;
					}
				}
				else 
				{
					SysTask.DClockStateSave = SysTask.DClockState;
					GPIO_ResetBits(LOCK_GPIO, LOCK_D_PIN);
                    SysTask.ClockD_offTime = 0;
				}
			}

			break;

		case CLOCK_STATE_DEBOUNSE:
			if (SysTask.ClockDStateTime == 0)
			{
				if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_D_PIN) == 0) // //检测到零点 开锁
				{
					GPIO_SetBits(LOCK_GPIO, LOCK_D_PIN);
					SysTask.ClockDTask	= CLOCK_STATE_DETECT;
				}
			}

			break;

		default:
			break;
	}


}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void MainTask(void)
{
	RemoteTask();
	FingerTouchTask();
	ClockTask();
	Moto_A_Task();
	Moto_B_Task();
	Moto_C_Task();
	Moto_D_Task();

	//SaveTask();
}



/*******************************************************************************
* 名称: 
* 功能: 填充表格
* 形参:		
* 返回: 无
* 说明: 三行4列
*		   |   |   |   |   |		
*		   | _ | _ | _ | _ |
*		   |   |   |   |   |		
*		   | _ | _ | _ | _ |
*		   |   |   |   |   |		
*		   | _ | _ | _ | _ |
*******************************************************************************/
void FillBox(u8 u8BoxIndex)
{
	unsigned char xStart = 0, xEnd = 0, yPageStart = 0, yPageEnd = 0, TopSize = 0, yPixSize = 0;

	yPixSize			= 21;

	xStart				= (u8BoxIndex % 4) * 32;
	xEnd				= xStart + 32;

	switch (u8BoxIndex)
	{
		case 0x00: //A格
		case 0x01: //B格
		case 0x02: //C格
		case 0x03: //D格
			yPageStart = 0;
			yPageEnd = 2;
			TopSize = 0;
			break;

		case 0x04: //
		case 0x05: //
		case 0x06: //
		case 0x07: //
			yPageStart = 2;
			yPageEnd = 5;
			TopSize = 4;
			break;

		case 0x08: //
		case 0x09: //
		case 0x0A: //
		case 0x0B: //
			yPageStart = 5;
			yPageEnd = 7;
			TopSize = 2;
			break;

		default:
			break;
	}

	OLED_DrawSolidBox(xStart, xEnd, yPageStart, yPageEnd, TopSize, yPixSize);
}


/*******************************************************************************
* 名称: 
* 功能: 填充INIT表格
* 形参:		
* 返回: 无
* 说明: 三行4列
*		   |   |   |   |   |		
*		   | _ | _ | _ | _ |
*		   |   |   |   |   |		
*		   | _ | _ | _ | _ |
*		   |   |   |   |   |		
*		   | _ | _ | _ | _ |
*******************************************************************************/
void FillInitBox(u8 u8BoxIndex)
{
	unsigned char xStart = 0, xEnd = 0, yPageStart = 0, yPageEnd = 0, TopSize = 0, yPixSize = 0;

	yPixSize			= 31;

	xStart				= (u8BoxIndex % 4) * 32;
	xEnd				= xStart + 32;

	switch (u8BoxIndex)
	{
		case 0x00: //A格
		case 0x01: //B格
		case 0x02: //C格
		case 0x03: //D格
			yPageStart = 0;
			yPageEnd = 2;
			TopSize = 0;
			break;

		default:
			break;
	}

	OLED_DrawSolidBox(xStart, xEnd, yPageStart, yPageEnd, TopSize, yPixSize);
}



/*******************************************************************************
* 名称: 
* 功能: 填充主页面表格 表格
* 形参:		
* 返回: 无
* 说明: 三行4列
*		   | A | B	| C  | D  |		
*******************************************************************************/
void FillMainBox(u8 u8BoxIndex)
{
	unsigned char xStart = 0, xEnd = 0, yPageStart = 0, yPageEnd = 3, TopSize = 0, yPixSize = 32;

	yPixSize			= 21;

	xStart				= (u8BoxIndex % 4) * 32;
	xEnd				= xStart + 32;

	OLED_DrawSolidBox(xStart, xEnd, yPageStart, yPageEnd, TopSize, yPixSize);
}


/*******************************************************************************
* 名称: 
* 功能: 选中小三角形
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SelectTriangleBox(u8 u8Index)
{
	unsigned char xStart = 0, yPage = 0;


	xStart				= (u8Index % 4) * 32;

	switch (u8Index)
	{
		case 0x00: //A格
		case 0x01: //B格
		case 0x02: //C格
		case 0x03: //D格
			yPage = 1;
			break;

		case 0x04: //
		case 0x05: //
		case 0x06: //
		case 0x07: //
			yPage = 3;
			break;

		case 0x08: //
		case 0x09: //
		case 0x0A: //
		case 0x0B: //
			yPage = 6;
			break;

		default:
			break;
	}

	xStart				= xStart + 2;
	OLED_DrawBMP(xStart, yPage, xStart + 8, yPage + 1, BMP_Select);
}


/*******************************************************************************
* 名称: 
* 功能: 虚线
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void ShowBox(void)
{
	u8 i;

	//	OLED_DrawHorizontal(0, 128, 2, 5, 1);			//实线
	//	OLED_DrawHorizontal(0, 128, 5, 2, 1);
	//	for (i = 1; i < 4; i++)
	//	{
	//		OLED_DrawVertical(32 * i, 0, 8, 1);
	//	}
	OLED_DrawHorizontalDottedLine(0, 128, 2, 5, 1); //虚线
	OLED_DrawHorizontalDottedLine(0, 128, 5, 2, 1);


	for (i = 1; i < 4; i++)
	{
		OLED_DrawVerticalDottedLine(32 * i, 0, 8, 1);
	}

}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
u8 CheckFlash(u16 * Read, u16 * Write, u16 Count)
{
	u8 i;

	for (i = 0; i < Count; i++)
	{
		if (*Read++ != *Write++)
			return 1;
	}

	return 0; //成功
}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void ShowLaunch(void)
{

	OLED_DrawBMP(0, 0, 128, 8, BMP_SanFeng);    

}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SysInit(void)
{
    u8 i; 
	FLASH_ReadMoreData(PASSWD_ADDR, SysTask.u16FlashPasswd, FLASH_READ_CNT); //读取密码
	if(SysTask.u16FlashPasswd[0] == 0xffff
        ||SysTask.u16FlashPasswd[1] == 0xffff
        ||SysTask.u16FlashPasswd[2] == 0xffff
        ||SysTask.u16FlashPasswd[3] == 0xffff
        ||SysTask.u16FlashPasswd[4] == 0xffff
        ||SysTask.u16FlashPasswd[5] == 0xffff
        ||SysTask.u16FlashPasswd[6] == 0xffff
        )
    {
        for(i = 0; i < FLASH_READ_CNT; i++)
            SysTask.u16FlashPasswd[i] = 0;

        
        FLASH_WriteMoreData(PASSWD_ADDR, SysTask.u16FlashPasswd, FLASH_READ_CNT);
    }   
    
	if (SysTask.u16FlashPasswd[FLASH_READ_CNT - 1] < 4)
	{
		FLASH_ReadMoreData(DATA_ADDR + SysTask.u16FlashPasswd[FLASH_READ_CNT - 1] * 1024, SysTask.u16FlashData, DATA_READ_CNT);

		//读取数据	
		SysTask.MotoATime	= (MotoTime)
		SysTask.u16FlashData[0];
		SysTask.MotoBTime	= (MotoTime)
		SysTask.u16FlashData[1];
		SysTask.MotoCTime	= (MotoTime)
		SysTask.u16FlashData[2];
		SysTask.MotoDTime	= (MotoTime)
		SysTask.u16FlashData[3];

		SysTask.MotoAMode	= (MotoFR)
		SysTask.u16FlashData[4];
		SysTask.MotoBMode	= (MotoFR)
		SysTask.u16FlashData[5];
		SysTask.MotoCMode	= (MotoFR)
		SysTask.u16FlashData[6];
		SysTask.MotoDMode	= (MotoFR)
		SysTask.u16FlashData[7];
	}
	else 
	{
		SysTask.MotoATime	= MOTO_TIME_1950;		//电机1运行时间 转2分停245S 后续改从flash读取保存的值
		SysTask.MotoBTime	= MOTO_TIME_1950;		//电机2运行时间
		SysTask.MotoCTime	= MOTO_TIME_1950;		//电机3运行时间
		SysTask.MotoDTime	= MOTO_TIME_1950;		//电机4运行时间


		SysTask.MotoAMode	= MOTO_FR_FWD_REV;		//后续改从flash读取保存的值
		SysTask.MotoBMode	= MOTO_FR_FWD;
		SysTask.MotoCMode	= MOTO_FR_FWD;
		SysTask.MotoDMode	= MOTO_FR_FWD;

	}

	SysTask.RemoteState = SAN_DEF;
	SysTask.RemoteSub	= INIT;
	SysTask.TouchState	= TOUCH_INIT;
	SysTask.TouchSub	= TOUCH_SUB_INIT;
	SysTask.nShowTime	= 0;
	SysTask.nSubWaitTime = 0;
	SysTask.nWaitTime	= 0;
	SysTask.nTick		= 0;
	SysTask.nLoadTime	= 2000; 					//2S Luanch时间
	SysTask.MotoAStateTime = 0;
	SysTask.MotoBStateTime = 0;
	SysTask.MotoCStateTime = 0;
	SysTask.MotoDStateTime = 0;

	SysTask.MotoChoose	= 0;

	SysTask.ClockATask	= CLOCK_STATE_DETECT;
	SysTask.ClockBTask	= CLOCK_STATE_DETECT;
	SysTask.ClockCTask	= CLOCK_STATE_DETECT;
	SysTask.ClockDTask	= CLOCK_STATE_DETECT;

	SysTask.AClockState = CLOCK_OFF;				//关闭状态	 
	SysTask.BClockState = CLOCK_OFF;				//关闭状态	 
	SysTask.CClockState = CLOCK_OFF;				//关闭状态	 
	SysTask.DClockState = CLOCK_OFF;				//关闭状态	 

	SysTask.AClockStateSave = CLOCK_OFF;			//关闭状态		
	SysTask.BClockStateSave = CLOCK_OFF;			//关闭状态		
	SysTask.CClockStateSave = CLOCK_OFF;			//关闭状态		
	SysTask.DClockStateSave = CLOCK_OFF;			//关闭状态	



	SysTask.MotoATimeSave = MOTO_TIME_OFF;			//电机1运行时间  save为0 保证开机起来会转
	SysTask.MotoBTimeSave = MOTO_TIME_OFF;			//电机2运行时间
	SysTask.MotoCTimeSave = MOTO_TIME_OFF;			//电机3运行时间
	SysTask.MotoDTimeSave = MOTO_TIME_OFF;			//电机4运行时间


	SysTask.MotoAModeSave = MOTO_FR_DEF;			//保证开机起来会转
	SysTask.MotoBModeSave = MOTO_FR_DEF;
	SysTask.MotoCModeSave = MOTO_FR_DEF;
	SysTask.MotoDModeSave = MOTO_FR_DEF;

	SysTask.MotoAState	= MOTO_STATE_INIT;			//电机1运行状态
	SysTask.MotoBState	= MOTO_STATE_INIT;			//电机2运行状态
	SysTask.MotoCState	= MOTO_STATE_INIT;			//电机3运行状态
	SysTask.MotoDState	= MOTO_STATE_INIT;			//电机4运行状态


	SysTask.MotoASubState = MOTO_SUB_STATE_RUN; 	//电机1子运行状态
	SysTask.MotoBSubState = MOTO_SUB_STATE_RUN; 	//电机2子运行状态
	SysTask.MotoCSubState = MOTO_SUB_STATE_RUN; 	//电机3子运行状态
	SysTask.MotoDSubState = MOTO_SUB_STATE_RUN; 	//电机4子运行状态

    SysTask.ClockA_offTime=0;
    SysTask.ClockB_offTime=0;
    SysTask.ClockC_offTime=0;
    SysTask.ClockD_offTime=0;


}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void ShowInit(void)
{
	OLED_Clear();
	OLED_DrawBMP(0, 0, 128, 8, BMP_Init);

	if ((SysTask.MotoAState == MOTO_STATE_STOP) || SysTask.MotoAState == MOTO_STATE_IDLE)
//		FillInitBox(0);
    OLED_DrawBMP(0, 0, 32, 4, BMP_fill);

	if ((SysTask.MotoBState == MOTO_STATE_STOP) || SysTask.MotoBState == MOTO_STATE_IDLE)
		//FillInitBox(1);
    OLED_DrawBMP(32, 0, 64, 4, BMP_fill);

	if ((SysTask.MotoCState == MOTO_STATE_STOP) || SysTask.MotoCState == MOTO_STATE_IDLE)
		//FillInitBox(2);
    OLED_DrawBMP(64, 0, 96, 4, BMP_fill);

	if ((SysTask.MotoDState == MOTO_STATE_STOP) || SysTask.MotoDState == MOTO_STATE_IDLE)
		//FillInitBox(3);
    OLED_DrawBMP(96, 0, 128, 4, BMP_fill);

	switch (SysTask.MotoAMode)
	{
		case MOTO_FR_FWD:
			OLED_ShowString(4, 5, "FWD", 12);
			break;

		case MOTO_FR_REV:
			OLED_ShowString(4, 5, "REV", 12);
			break;

		case MOTO_FR_FWD_REV:
			OLED_ShowString(4, 5, "F-R", 12);
			break;

		default:
			break;
	}

	switch (SysTask.MotoBMode)
	{
		case MOTO_FR_FWD:
			OLED_ShowString(36, 5, "FWD", 12);
			break;

		case MOTO_FR_REV:
			OLED_ShowString(36, 5, "REV", 12);
			break;

		case MOTO_FR_FWD_REV:
			OLED_ShowString(36, 5, "F-R", 12);
			break;

		default:
			break;
	}

	switch (SysTask.MotoCMode)
	{
		case MOTO_FR_FWD:
			OLED_ShowString(68, 5, "FWD", 12);
			break;

		case MOTO_FR_REV:
			OLED_ShowString(68, 5, "REV", 12);
			break;

		case MOTO_FR_FWD_REV:
			OLED_ShowString(68, 5, "F-R", 12);
			break;

		default:
			break;
	}

	switch (SysTask.MotoDMode)
	{
		case MOTO_FR_FWD:
			OLED_ShowString(100, 5, "FWD", 12);
			break;

		case MOTO_FR_REV:
			OLED_ShowString(100, 5, "REV", 12);
			break;

		case MOTO_FR_FWD_REV:
			OLED_ShowString(100, 5, "F-R", 12);
			break;

		default:
			break;
	}

	switch (SysTask.MotoATime)
	{
		case MOTO_TIME_TPD:
			OLED_ShowString(0, 7, "TPD", 12);
			break;

		case MOTO_TIME_650:
			OLED_ShowString(0, 7, "650", 12);
			break;

		case MOTO_TIME_750:
			OLED_ShowString(0, 7, "750", 12);
		case MOTO_TIME_850:
			OLED_ShowString(0, 7, "850", 12);
		case MOTO_TIME_1000:
			OLED_ShowString(0, 7, "1000", 12);
		case MOTO_TIME_1950:
			OLED_ShowString(0, 7, "1950", 12);
			break;

		default:
			break;
	}

	switch (SysTask.MotoBTime)
	{
		case MOTO_TIME_TPD:
			OLED_ShowString(32, 7, "TPD", 12);
			break;

		case MOTO_TIME_650:
			OLED_ShowString(32, 7, "650", 12);
			break;

		case MOTO_TIME_750:
			OLED_ShowString(32, 7, "750", 12);
		case MOTO_TIME_850:
			OLED_ShowString(32, 7, "850", 12);
		case MOTO_TIME_1000:
			OLED_ShowString(32, 7, "1000", 12);
		case MOTO_TIME_1950:
			OLED_ShowString(32, 7, "1950", 12);
			break;

		default:
			break;
	}

	switch (SysTask.MotoCTime)
	{
		case MOTO_TIME_TPD:
			OLED_ShowString(64, 7, "TPD", 12);
			break;

		case MOTO_TIME_650:
			OLED_ShowString(64, 7, "650", 12);
			break;

		case MOTO_TIME_750:
			OLED_ShowString(64, 7, "750", 12);
		break;
		case MOTO_TIME_850:
			OLED_ShowString(64, 7, "850", 12);
		break;
		case MOTO_TIME_1000:
			OLED_ShowString(64, 7, "1000", 12);
		break;
		case MOTO_TIME_1950:
			OLED_ShowString(64, 7, "1950", 12);
			break;

		default:
			break;
	}

	switch (SysTask.MotoDTime)
	{
		case MOTO_TIME_TPD:
			OLED_ShowString(96, 7, "TPD", 12);
			break;

		case MOTO_TIME_650:
			OLED_ShowString(96, 7, "650", 12);
			break;

		case MOTO_TIME_750:
			OLED_ShowString(96, 7, "750", 12);
		break;
		case MOTO_TIME_850:
			OLED_ShowString(96, 7, "850", 12);
		break;
		case MOTO_TIME_1000:
			OLED_ShowString(96, 7, "1000", 12);
		break;
		case MOTO_TIME_1950:
			OLED_ShowString(96, 7, "1950", 12);
			break;

		default:
			break;
	}

}


/*******************************************************************************
* 名称: 
* 功能: 
* 形参:		
* 返回: 无
* 说明: 
*******************************************************************************/
void SaveData(void)
{
	u16 u16ReadData[DATA_READ_CNT] =
	{
		0
	};
	s8 i, j, Result 	= -1;

	if ((SysTask.MotoATime != SysTask.u16FlashData[0]) || (SysTask.MotoBTime != SysTask.u16FlashData[1]) ||
		 (SysTask.MotoCTime != SysTask.u16FlashData[2]) || (SysTask.MotoDTime != SysTask.u16FlashData[3]) ||
		 (SysTask.MotoAMode != SysTask.u16FlashData[4]) || (SysTask.MotoBMode != SysTask.u16FlashData[5]) ||
		 (SysTask.MotoCMode != SysTask.u16FlashData[6]) || (SysTask.MotoDMode != SysTask.u16FlashData[7])) //如果数据改变则保存
	{
		SysTask.u16FlashData[0] = SysTask.MotoATime;
		SysTask.u16FlashData[1] = SysTask.MotoBTime;
		SysTask.u16FlashData[2] = SysTask.MotoCTime;
		SysTask.u16FlashData[3] = SysTask.MotoDTime;
		SysTask.u16FlashData[4] = SysTask.MotoAMode;
		SysTask.u16FlashData[5] = SysTask.MotoBMode;
		SysTask.u16FlashData[6] = SysTask.MotoCMode;
		SysTask.u16FlashData[7] = SysTask.MotoDMode;

		for (i = 0; i < 3; i++)
		{
			Result	= FLASH_WriteMoreData(DATA_ADDR 
                                          + SysTask.u16FlashPasswd[FLASH_READ_CNT - 1] *1024, 
                                          SysTask.u16FlashData,
                                          DATA_READ_CNT);

			if (Result == 0)
				break;
		}

		FLASH_ReadMoreData(DATA_ADDR + SysTask.u16FlashPasswd[FLASH_READ_CNT - 1] * 1024, u16ReadData, DATA_READ_CNT);

		if (CheckFlash(u16ReadData, SysTask.u16FlashData, DATA_READ_CNT)) //读取出来的与写入的不一致重新换另一页写入
		{
			for (i = 0; i < 3; i++)
			{
				//换另一页写入
				for (i = 1; i < 3 - SysTask.u16FlashPasswd[FLASH_READ_CNT - 1]; i++)
				{
					for (i = 0; i < 3; i++)
					{
						Result				=
							 FLASH_WriteMoreData(DATA_ADDR + SysTask.u16FlashPasswd[FLASH_READ_CNT - 1] *1024, SysTask.u16FlashData, DATA_READ_CNT);

						if (Result == 0)
							break;
					}

					FLASH_ReadMoreData(DATA_ADDR + SysTask.u16FlashPasswd[FLASH_READ_CNT - 1]*1024, u16ReadData, DATA_READ_CNT);

					if (Result == 0)
					{
						SysTask.u16FlashPasswd[FLASH_READ_CNT - 1] = i;

						//刷新存储位置信息
						for (j = 0; j < 3; j++)
						{

							Result				=
								 FLASH_WriteMoreData(PASSWD_ADDR, SysTask.u16FlashPasswd, FLASH_READ_CNT);

							if (Result == 0)
								break;
						}

						if (j == 3)
							OLED_ShowString(30, 7, "flash err", 12);

						break;
					}
				}
			}
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
void OledInitTask(void)
{
	SysTask.RemoteState = SAN_DEF;
	SysTask.RemoteSub	= INIT;
	SysTask.nShowTime	= 0;
	SysTask.TouchState	= TOUCH_INIT;
	SysTask.TouchSub	= TOUCH_SUB_INIT;
	SysTask.MotoChoose	= 0;
	SysTask.MotoAStateTime = 0;
	SysTask.MotoBStateTime = 0;
	SysTask.MotoCStateTime = 0;
	SysTask.MotoDStateTime = 0;

	SysTask.ClockAStateTime = 0;
	SysTask.ClockBStateTime = 0;
	SysTask.ClockCStateTime = 0;
	SysTask.ClockDStateTime = 0;

	SysTask.AClockState = CLOCK_OFF;				//关闭状态	 
	SysTask.BClockState = CLOCK_OFF;				//关闭状态	 
	SysTask.CClockState = CLOCK_OFF;				//关闭状态	 
	SysTask.DClockState = CLOCK_OFF;				//关闭状态	 

	SysTask.AClockStateSave = CLOCK_OFF;			//关闭状态		
	SysTask.BClockStateSave = CLOCK_OFF;			//关闭状态		
	SysTask.CClockStateSave = CLOCK_OFF;			//关闭状态		
	SysTask.DClockStateSave = CLOCK_OFF;			//关闭状态	

	GPIO_ResetBits(LOCK_GPIO, LOCK_A_PIN | LOCK_B_PIN | LOCK_C_PIN | LOCK_D_PIN);
	ShowInit();
	SaveData();
}


