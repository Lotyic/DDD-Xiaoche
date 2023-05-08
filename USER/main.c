#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "oled.h"
#include "oled2.h"
#include "timer.h"
#include "gpio.h"
#include "ADC_Config.h"
#include "moto.h"
#include "move.h"
#include "st178.h"
#include "ui.h"
#include "exti.h"
#include "stmflash.h"


u16 TLHXBCS=0;
int main(void)
{
	delay_init(); //延时初始化
	OLED_Init();  // OLED初始化
	LCD_CLS();	  // OLED清屏
	// TZXH_LOGO();//台州学院logo显示
	All_GPIO_Init();  //用到的IO口初始化
	uart1_init(115200); //串口1
	uart2_init(115200); //串口2
	uart3_init(9600); //串口3

	Init_BMQ(); //编码器初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	ADC_DMA_Configuration(); // ADC1初始化(不知道有没有用到，上车调试)
	ADKey_Init();			 // ACD3初始化(用到)

	Init_TIM8_PWM(899, 7); //电机PWM波

	Init_TIM1_PWM(19999, 71); // pe11 pe13舵机(预留)(周期20ms)
	// Init_TIM6_PWM(4, 71);	  //机械臂舵机10ms一次

	
	// zuodongzuo(100, 0);
	
	read_from_flash();		  //从flash中读取数据							
	LCD_CLS();

//printf("%s","TLCDB!");
//		zuodongzuo(100, 1);
	delay_ms(10);
	printf("%s\r\n","TLCDB!");	//实验室精灵唐林吃个大便就能解决双车蓝牙通信问题
	delay_ms(10);
	angle_cs=angle_6;
	while (1)
	{		
		// sprintf(OLED_BUF,"%d 	",ADKey_Deal(Get_Key_Adc_Average(ADC_Channel_4,1)));//显示
		// LCD_16_HanZi_ASCII(0,6,OLED_BUF);	

	    MenuOperate();
//		delay_ms(1000);
//		delay_ms(1000);
//		zuodongzuo(2, 1);
//		delay_ms(1000);
	}
}
