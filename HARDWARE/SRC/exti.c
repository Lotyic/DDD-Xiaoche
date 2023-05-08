#include "exti.h"
#include "timer.h"
#include "sys.h"
#include "st178.h"
#include "moto.h"
#include "ui.h"

char cunchu[4] = {0, 0, 0, 0};
char z_cha = 0, y_cha = 0, q_cha = 0, h_cha = 0;
u16 count_fz = 0;
u8 fz_flag = 0;

u16 bb12 = 70;
u16 bb13 = 70;
u16 bb14 = 150;

u16 b12 = 0;
u16 b13 = 0;
u16 b14 = 0;

//黄板子 b12小->大->逆时针
//黄板子 b13小->大->顺时针

u16 dongzuo[][3] =
	{
		{350, 100, 295}, //复位0
		
		{252, 395, 200}, //三分抓松1
		
		{252, 395, 295}, //三分抓紧2
		{250, 310, 295}, //过渡3
		{350, 100, 295}, //后放4
		
		{380, 460, 295}, //三分放紧5
		{380, 460, 295}, //三分放紧56
		{380, 460, 295}, //三分放紧57
		{380, 460, 200}, //三分放松68
		{350, 100, 200}, //复位79
		
		{163, 310, 200}, //两分抓前放8
		
		{163, 310, 295}, //两分抓紧9
		{350, 100, 295}, //后放10
		
		{380, 520, 295}, //两分放紧11
		{380, 520, 295}, //两分放紧11
		{380, 520, 295}, //两分放紧11
		{380, 520, 200}, //两分放松12
		{350, 100, 200}//复位13
};
u16 dongshuzu[][2] = {{0, 0},{1,1}, {2, 4}, {5, 9}, {10,10},{11, 12}, {13, 17}}; // 0复位0 1放1，3-4三分抓(9cm)2 5-7三分放3 8-8放4，10-11二分抓(11.5cm)5 12-15二分放6
void fzopen()
{
	cunchu[0] = zuo_sum;
	cunchu[1] = you_sum;
	cunchu[2] = qian_sum;
	cunchu[3] = hou_sum;

	Init_TIM6(99, 7199);
}

void fzclose()
{
	TIM_Cmd(TIM6, DISABLE);
}

void Init_TIM6(u16 arr, u16 psc)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr;						//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数模式
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);				//根据指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;			  // TIM7中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);							  //初始化NVIC寄存器

	TIM_Cmd(TIM6, ENABLE);
}

void TIM6_IRQHandler(void)
{
	// 10us一次
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		b12 %= 2000;
		b12++;
		if (b12 <= bb12)
			PBout(12) = 1;
		else
			PBout(12) = 0;

		b13 %= 2000;
		b13++;
		if (b13 <= bb13)
			PBout(13) = 1;
		else
			PBout(13) = 0;

		b14 %= 2000;
		b14++;
		if (b14 <= bb14)
			PBout(14) = 1;
		else
			PBout(14) = 0;
	}

	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	//	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	//	{
	//		z_cha=zuo_sum-cunchu[0];
	//		if(z_cha<0)
	//			z_cha=-z_cha;
	//		y_cha=you_sum-cunchu[1];
	//		if(y_cha<0)
	//			y_cha=-y_cha;
	//		q_cha=qian_sum-cunchu[2];
	//		if(q_cha<0)
	//			q_cha=-q_cha;
	//		h_cha=hou_sum-cunchu[3];
	//		if(h_cha<0)
	//			h_cha=-h_cha;
	//		if(q_cha>0||h_cha>0 ||z_cha>0||y_cha>0)
	//		{
	//			count_fz=0;
	//			cunchu[0]=zuo_sum;
	//			cunchu[1]=you_sum;
	//			cunchu[2]=qian_sum;
	//			cunchu[3]=hou_sum;
	//		}
	//		else
	//		{
	//			count_fz++;
	//		}
	//		if(count_fz>=300)
	//		{
	//			fz_flag=1;
	//			stop();
	//			count_fz=0;
	//		}
	//
	//		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
	//	}
}
void zuodongzuo(u8 xuhao, u8 yanshi)
{
	int iii;
	int iiii;
	if (xuhao < 100)
	{
		TIM_Cmd(TIM6, ENABLE);
		for (iii = dongshuzu[xuhao][0]; iii <= dongshuzu[xuhao][1]; iii++)
		{
			delay_ms(10);
			bb12 = dongzuo[iii][0];
			delay_ms(10);
			bb13 = dongzuo[iii][1];
			delay_ms(10);
			bb14 = dongzuo[iii][2];
			for (iiii = 0; iiii < yanshi*4; iiii++)
				delay_ms(100);
		}
		delay_ms(600);
//		TIM_Cmd(TIM6, DISABLE);
	}
	else
	{
		TIM_Cmd(TIM1, ENABLE);
		if (xuhao == 100) //关闭
		{
				TIM_SetCompare2(TIM1, 1660); //左舵机PE11   20000为满pwm   左1接左边舵机  变小->变大
				delay_ms(15);	
				TIM_SetCompare3(TIM1, 1320); //右舵机PE13               左3接右边舵机1530  变小->变小
				delay_ms(10);
//			TIM_SetCompare2(TIM1, 1560); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(5);
//			TIM_SetCompare3(TIM1, 1530); //右舵机PE13               左3接右边舵机1530
//			delay_ms(10);
		}
		else if (xuhao == 101) //保护环1个
		{
//			  TIM_SetCompare2(TIM1, 1550); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//				delay_ms(5);	
//				TIM_SetCompare3(TIM1, 1530); //右舵机PE13               左3接右边舵机1530
//				delay_ms(10);
			TIM_SetCompare2(TIM1, 1170); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
			delay_ms(5);
			TIM_SetCompare3(TIM1, 1790); //右舵机PE13               左3接右边舵机1530
			delay_ms(10);
		}
		else if (xuhao == 102) //打开中
		{
				TIM_SetCompare2(TIM1, 800);
				delay_ms(2);
				TIM_SetCompare3(TIM1, 2100); 
				delay_ms(10);
//			TIM_SetCompare2(TIM1, 850); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(10);
//			TIM_SetCompare3(TIM1, 2250); //右舵机PE13               左3接右边舵机
		}
		else if (xuhao == 103) //打开2最大
		{
				TIM_SetCompare2(TIM1, 500);
				delay_ms(2);
				TIM_SetCompare3(TIM1, 2500); 
				delay_ms(10);
//			TIM_SetCompare2(TIM1, 850); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(10);
//			TIM_SetCompare3(TIM1, 2250); //右舵机PE13               左3接右边舵机
		}
		else if (xuhao == 104) //直角
		{
			TIM_SetCompare2(TIM1, 870); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
			delay_ms(5);
			TIM_SetCompare3(TIM1, 2000); //右舵机PE13               左3接右边舵机1530
			delay_ms(10);
//			TIM_SetCompare2(TIM1, 850); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(10);
//			TIM_SetCompare3(TIM1, 2250); //右舵机PE13               左3接右边舵机
		}
		else if (xuhao == 105) //半开
		{
			TIM_SetCompare2(TIM1, 500); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
			delay_ms(5);
			TIM_SetCompare3(TIM1, 2100); //右舵机PE13               左3接右边舵机1530
			delay_ms(10);
//			TIM_SetCompare2(TIM1, 850); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(10);
//			TIM_SetCompare3(TIM1, 2250); //右舵机PE13               左3接右边舵机
		}
		else if (xuhao == 106) //右垂直
		{
			TIM_SetCompare2(TIM1, 500); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
			delay_ms(5);
			TIM_SetCompare3(TIM1, 2000); //右舵机PE13               左3接右边舵机1530
			delay_ms(10);
//			TIM_SetCompare2(TIM1, 850); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(10);
//			TIM_SetCompare3(TIM1, 2250); //右舵机PE13               左3接右边舵机
		}
		else if (xuhao == 107) //半开
		{
			TIM_SetCompare2(TIM1, 870); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
			delay_ms(5);
			TIM_SetCompare3(TIM1, 2500); //右舵机PE13               左3接右边舵机1530
			delay_ms(10);
//			TIM_SetCompare2(TIM1, 850); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(10);
//			TIM_SetCompare3(TIM1, 2250); //右舵机PE13               左3接右边舵机
		}
		else if (xuhao == 108) //左垂直
		{
			TIM_SetCompare2(TIM1, 1000); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
			delay_ms(5);
			TIM_SetCompare3(TIM1, 2500); //右舵机PE13               左3接右边舵机1530
			delay_ms(10);
//			TIM_SetCompare2(TIM1, 850); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(10);
//			TIM_SetCompare3(TIM1, 2250); //右舵机PE13               左3接右边舵机
		}
		else if (xuhao == 109) //打开角落
		{
			TIM_SetCompare2(TIM1, 650); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
			delay_ms(5);
			TIM_SetCompare3(TIM1, 2300); //右舵机PE13               左3接右边舵机1530
			delay_ms(10);
//			TIM_SetCompare2(TIM1, 850); //左舵机PE11   20000为满pwm   左1接左边舵机  1560
//			delay_ms(10);
//			TIM_SetCompare3(TIM1, 2250); //右舵机PE13               左3接右边舵机
		}
		for (iiii = 0; iiii < yanshi*4; iiii++)
		{
			delay_ms(100);
		}
		delay_ms(600);
//		TIM_Cmd(TIM1, DISABLE);
	}
}