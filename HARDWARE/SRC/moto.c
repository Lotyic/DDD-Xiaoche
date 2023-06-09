#include "moto.h"
#include "exti.h"

u8 turn_af = 0;
u16 dl=0;

int ii = 0;

s16 tt;//读取前后的循迹返回是经过了几个路口
//s8  aa;//读取左右循迹值返回是否停车准确

u8 FX = 1;
u32 pwm_time=0;
u16 sum_flag=0;
/*编码器移动需要用到的参数*/
short int sum_value=0;
u8 pidflag=1;
short int sum_value1=0;
short int sum_value2=0;
short int sum_cha=0;
float angle_cs;
float KP=9.5;//9.5
float KI=10;//10  
float KD=2;//2

u8 stop_TIM = 1;		//停车标志位
u8 tim7_start=0;		//pid调节标志位

// u8 velocity_PID_KP_qh[5] = {0,55,58,59,91};
// u8 velocity_PID_KD_qh[5] = {0,55,69,75,118};


// u8 velocity_PID_KP_qh[5] = {0,35,43,49,71};
// u8 velocity_PID_KD_qh[5] = {0,35,54,65,98};

u8 velocity_PID_KP_qh[5] = {0,65,73,79,55};
u8 velocity_PID_KD_qh[5] = {0,65,84,95,55};


/*----------------------------计数部分--------------------------*/

u8 g_nInterrupt_Count=0;   //10ms

float velocity_KP=56;
float velocity_KI=0;
float velocity_KD=50;//32

float Encoder_Integral;

struct PID Pid_fl={0,0,0,0,0,0,0,0,0,0,0};
struct PID Pid_fr={0,0,0,0,0,0,0,0,0,0,0};
struct PID Pid_br={0,0,0,0,0,0,0,0,0,0,0};
struct PID Pid_bl={0,0,0,0,0,0,0,0,0,0,0};

int ll_PWM = 0;
int lr_PWM = 0;
int bl_PWM = 0;
int br_PWM = 0;

int speed=0;
u8 speed_carrage[10]={0,25,35,60,20,18};

int16_t a_pwm[6];

///*/////////////////////////////////////////

//  LL            LR


//         ↑

//  BL            BR

///*/////////////////////////////////////////*/

void stop(void)
{	
	
//	printf("%d\n",pwm_time);

	tim7_start = 0;
	
	tt = 0;
	//aa = 0;
	
	pwm_time = 0;
	
	stop_TIM = 0;
	
	Pid_fl.set=0;
	Pid_fr.set=0;	
	Pid_bl.set=0;
	Pid_br.set=0;

	TIM_SetCompare1(TIM8,0);
	TIM_SetCompare2(TIM8,0);
	TIM_SetCompare3(TIM8,0);
	TIM_SetCompare4(TIM8,0);
	
	bxcls();	
	TIM_Cmd(TIM7, DISABLE);
//	fzclose();                    //防撞关闭
	
	
	
	
}
	
void PWM_PID(struct PID *pid_s)
{
	(*pid_s).det=(*pid_s).set-(*pid_s).sample;
	(*pid_s).pwm=(*pid_s).pwm+(*pid_s).kp*((*pid_s).det-(*pid_s).pre_det)+(*pid_s).ki*(*pid_s).det+(*pid_s).kd*((*pid_s).det-2*(*pid_s).pre_det+(*pid_s).last_det);
	(*pid_s).last_det=(*pid_s).pre_det;
	(*pid_s).pre_det=(*pid_s).det;
	
	if((*pid_s).pwm>=830)
	{
		(*pid_s).pwm=830;
	}
	if((*pid_s).pwm<=-830)
	{
		(*pid_s).pwm=-830; 
	}
}

void XIANFU(struct PID *pid_s)
{
	
	if((*pid_s).pwm>=850)
	{
		(*pid_s).pwm=850;
	}
	if((*pid_s).pwm<=-850)
	{
		(*pid_s).pwm=-850; 
	}
}

int WEI_PID(void)     //微调PWM函数
{
	static float Encoder_Least,Encoder;
	int pwm;
	
	if(BX_FLAGE == 1)   //变向flag
		return 0;

	if(FX == 1)
	{
		velocity_KP = velocity_PID_KP_qh[speed];
		velocity_KD = velocity_PID_KD_qh[speed];
		
		Encoder_Least =(qian_sum+0)-0;
		
		chazhi_qh = -chazhi_qh;
		
		if(Encoder_Least>=3||Encoder_Least<=-3)
		{
			Encoder *= 0.8;		                                                //===一阶低通滤波器       
			Encoder += Encoder_Least*0.2;	                                    //===一阶低通滤波器    
		}
		else if(Encoder_Least>-3 && Encoder_Least<3)
		{
			Encoder = 0;
			Encoder_Integral = 0;
			chazhi_qh = 0;
		}
		pwm = Encoder*velocity_KP+Encoder_Integral*velocity_KI+chazhi_qh*velocity_KD;        //===速度控制	
		pwm = pwm/20;
		
	}
	else if(0 == FX)
	{
		velocity_KP = velocity_PID_KP_qh[speed];
		velocity_KD = velocity_PID_KD_qh[speed];
		
		Encoder_Least =(0+hou_sum)-0;
		
		chazhi_qh = -chazhi_qh;
		
		if(Encoder_Least>=3||Encoder_Least<=-3)
		{
			Encoder *= 0.8;		                                                //===一阶低通滤波器       
			Encoder += Encoder_Least*0.2;	                                    //===一阶低通滤波器    
		}
		else if(Encoder_Least>-3 && Encoder_Least<3)
		{
			Encoder = 0;
			Encoder_Integral = 0;
			chazhi_qh = 0;
		}
		pwm = Encoder*velocity_KP+Encoder_Integral*velocity_KI+chazhi_qh*velocity_KD;        //===速度控制	
		pwm = pwm/20;
	}
	
/*双车存储数据、发送数据*/
//		a_pwm[0]=hou_sum*100;
//		a_pwm[1]=qian_sum*100;
//		a_pwm[2]=chazhi_qh*100;
//		a_pwm[4]=pwm;

//		SentDateTo_usart(a_pwm);
	
	return pwm;
}


void SentDateTo_usart(int16_t *wareaddr)
{
  	u8 i;
   u8 DB[2]={0x03,0xFC};
   u8 DB1[2]={0XFC,0X03};
  for(i=0;i<2;i++)
  {
			 USART2->DR=DB[i];
		 while((USART2->SR&0x40)==0);	 
  }
			USART2->DR =wareaddr[0];
  	  while((USART2->SR&0x40)==0);
				USART2->DR =wareaddr[0]>>8;
  	  while((USART2->SR&0x40)==0);
				USART2->DR =wareaddr[1];
  	  while((USART2->SR&0x40)==0);
				USART2->DR =wareaddr[1]>>8;
  	  while((USART2->SR&0x40)==0);
	
			USART2->DR =wareaddr[2];
  	  while((USART2->SR&0x40)==0);
				USART2->DR =wareaddr[2]>>8;
  	  while((USART2->SR&0x40)==0);

			USART2->DR =wareaddr[3];
  	  while((USART2->SR&0x40)==0);
				USART2->DR =wareaddr[3]>>8;
  	  while((USART2->SR&0x40)==0);
	
			USART2->DR =wareaddr[4];
  	  while((USART2->SR&0x40)==0);
				USART2->DR =wareaddr[4]>>8;
  	  while((USART2->SR&0x40)==0);
			
						USART2->DR =wareaddr[5];
  	  while((USART2->SR&0x40)==0);
				USART2->DR =wareaddr[5]>>8;
  	  while((USART2->SR&0x40)==0);

   for(i=0;i<2;i++)
  {
			 USART2->DR=DB1[i];
		 while((USART2->SR&0x40)==0);	 
  }

}
void TIM7_BaseTime_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); //时钟使能

	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
	
	TIM_Cmd(TIM7, ENABLE); 
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;  //TIM7中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器

 //使能TIMx					
	
}

void TIM7_IRQHandler(void)      //调整轮子PID、PWM定时器7中断函数
{
	
	if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{		
//		AD_AnJian();		
		if(tim7_start)
		{
			/*读取四个轮子当时的PID参考值*/
			Pid_fl.sample =-((s16)TIM2->CNT);//5
			Pid_bl.sample =-((s16)TIM4->CNT);
			Pid_fr.sample =((s16)TIM3->CNT);
			Pid_br.sample =-((s16)TIM5->CNT);
			if(sum_flag)                    //编码器转弯计数
			{
				sum_value+=Pid_fr.sample;
				sum_value1+=Pid_fl.sample;//
				sum_value2+=Pid_bl.sample;//
				sum_cha=abs(sum_value1)-abs(sum_value2);
			}
			
			// sprintf(OLED_BUF,"%.0f 	",Pid_bl.sample);//显示
			// LCD_16_HanZi_ASCII(0,6,OLED_BUF);	
			// sprintf(OLED_BUF,"%.0f  ",Pid_br.sample);
			// LCD_16_HanZi_ASCII(8,6,OLED_BUF);				
			// sprintf(OLED_BUF,"%.0f  ",Pid_fl.sample);
			// LCD_16_HanZi_ASCII(0,4,OLED_BUF);				
			// sprintf(OLED_BUF,"%.0f  ",Pid_fr.sample);
			// LCD_16_HanZi_ASCII(8,4,OLED_BUF);	
			
			TIM2->CNT=0;
			TIM3->CNT=0;
			TIM4->CNT=0;
			TIM5->CNT=0;
					
			//aa = St178_Scanf(1);
			tt = St188_Scanf(1);
			
//			sprintf(OLED_BUF,"%d  %d  ",aa,speed);
//			LCD_16_HanZi_ASCII(0,4,OLED_BUF);

			if(turn_af==1)
			  {
				dl++;
				if(dl==30)
				{turn_af=0;dl=0;}
			  }




			
			if(FX == 1)				//直走
			{			
				/*设定四个轮子PID参考值*/
				Pid_fl.set=speed_carrage[speed]; 
				Pid_bl.set=speed_carrage[speed];
				Pid_fr.set=speed_carrage[speed];
				Pid_br.set=speed_carrage[speed];
				
				
//				sprintf(OLED_BUF,"%.f 	",Pid_fl.set);
//				LCD_16_HanZi_ASCII(0,4,OLED_BUF);	
//				sprintf(OLED_BUF,"%.f  ",Pid_fr.set);
//				LCD_16_HanZi_ASCII(8,4,OLED_BUF);				
//				sprintf(OLED_BUF,"%.f  ",Pid_bl.set);
//				LCD_16_HanZi_ASCII(0,6,OLED_BUF);				
//				sprintf(OLED_BUF,"%.f  ",Pid_br.set);
//				LCD_16_HanZi_ASCII(8,6,OLED_BUF);	
			/*调整用的PWM波*/
				Pid_fl.wei_pwm = WEI_PID();
				Pid_bl.wei_pwm = Pid_fl.wei_pwm ;
				Pid_fr.wei_pwm = Pid_fl.wei_pwm ;
				Pid_br.wei_pwm = Pid_fl.wei_pwm ;
				
				PWM_PID(&Pid_fl);
				PWM_PID(&Pid_fr);
				PWM_PID(&Pid_bl);
				PWM_PID(&Pid_br);
				
				if(pidflag)//是否用编码器标志位
				{
					Pid_fl.pwm = Pid_fl.pwm - Pid_fl.wei_pwm;
					Pid_bl.pwm = Pid_bl.pwm - Pid_bl.wei_pwm;
					Pid_fr.pwm = Pid_fr.pwm + Pid_fr.wei_pwm;
					Pid_br.pwm = Pid_br.pwm + Pid_br.wei_pwm;
				}
				else
				{
					Pid_fl.pwm = Pid_fl.pwm - sum_cha*0.1;
					Pid_bl.pwm = Pid_bl.pwm - sum_cha*0.1;
					Pid_fr.pwm = Pid_fr.pwm + sum_cha*0.1;
					Pid_br.pwm = Pid_br.pwm + sum_cha*0.1;
				}


//				sprintf(OLED_BUF,"%d 	",Pid_fl.pwm);
//				LCD_16_HanZi_ASCII(0,4,OLED_BUF);	
//				sprintf(OLED_BUF,"%d  ",Pid_fr.pwm);
//				LCD_16_HanZi_ASCII(8,4,OLED_BUF);				
//				sprintf(OLED_BUF,"%d  ",Pid_bl.pwm);
//				LCD_16_HanZi_ASCII(0,6,OLED_BUF);				
//				sprintf(OLED_BUF,"%d  %d",Pid_br.pwm,speed);
//				LCD_16_HanZi_ASCII(8,6,OLED_BUF);	

//				if(Pid_fl.pwm<0)	Pid_fl.pwm = 0;
//				if(Pid_bl.pwm<0)	Pid_bl.pwm = 0;
//				if(Pid_fr.pwm<0)	Pid_fr.pwm = 0;
//				if(Pid_br.pwm<0)	Pid_br.pwm = 0;

				
			}
			else if(FX == 0||FX==9)		//后退
			{
				Pid_fl.set=-speed_carrage[speed]; 
				Pid_bl.set=-speed_carrage[speed];
				Pid_fr.set=-speed_carrage[speed];
				Pid_br.set=-speed_carrage[speed];
								
				Pid_fl.wei_pwm = WEI_PID();
				Pid_bl.wei_pwm = Pid_fl.wei_pwm ;
				Pid_fr.wei_pwm = Pid_fl.wei_pwm ;
				Pid_br.wei_pwm = Pid_fl.wei_pwm ;

				PWM_PID(&Pid_fl);
				PWM_PID(&Pid_fr);
				PWM_PID(&Pid_bl);
				PWM_PID(&Pid_br);
					
				if(pidflag)
				{
					Pid_fl.pwm = Pid_fl.pwm - Pid_fl.wei_pwm;
					Pid_bl.pwm = Pid_bl.pwm - Pid_bl.wei_pwm;
					Pid_fr.pwm = Pid_fr.pwm + Pid_fr.wei_pwm;
					Pid_br.pwm = Pid_br.pwm + Pid_br.wei_pwm;
				}
				else
				{
					Pid_fl.pwm = Pid_fl.pwm - sum_cha*0.1;
					Pid_bl.pwm = Pid_bl.pwm - sum_cha*0.1;
					Pid_fr.pwm = Pid_fr.pwm + sum_cha*0.1;
					Pid_br.pwm = Pid_br.pwm + sum_cha*0.1;
				}
				
//				if(Pid_fl.pwm>0)	Pid_fl.pwm = 0;
//				if(Pid_bl.pwm>0)	Pid_bl.pwm = 0;
//				if(Pid_fr.pwm>0)	Pid_fr.pwm = 0;
//				if(Pid_br.pwm>0)	Pid_br.pwm = 0;
			}
			else if(FX == 2)		//左转
			{
				Pid_fl.set=-speed_carrage[speed]; 
				Pid_bl.set=-speed_carrage[speed];
				Pid_fr.set=speed_carrage[speed];
				Pid_br.set=speed_carrage[speed];
			
				Pid_fl.wei_pwm = 0;
				Pid_bl.wei_pwm = 0;
				Pid_fr.wei_pwm = 0;
				Pid_br.wei_pwm = 0;
				
				PWM_PID(&Pid_fl);
				PWM_PID(&Pid_fr);
				PWM_PID(&Pid_bl);
				PWM_PID(&Pid_br);
				
				Pid_fl.pwm = Pid_fl.pwm - Pid_fl.wei_pwm;
				Pid_bl.pwm = Pid_bl.pwm - Pid_bl.wei_pwm;
				Pid_fr.pwm = Pid_fr.pwm + Pid_fr.wei_pwm;
				Pid_br.pwm = Pid_br.pwm + Pid_br.wei_pwm;
			}
			else if(FX == 3)		//左转
			{
				Pid_fl.set=speed_carrage[speed]; 
				Pid_bl.set=speed_carrage[speed];
				Pid_fr.set=-speed_carrage[speed];
				Pid_br.set=-speed_carrage[speed];
			
				Pid_fl.wei_pwm = 0;
				Pid_bl.wei_pwm = 0;
				Pid_fr.wei_pwm = 0;
				Pid_br.wei_pwm = 0;
				
				PWM_PID(&Pid_fl);
				PWM_PID(&Pid_fr);
				PWM_PID(&Pid_bl);
				PWM_PID(&Pid_br);
				
				Pid_fl.pwm = Pid_fl.pwm - Pid_fl.wei_pwm;
				Pid_bl.pwm = Pid_bl.pwm - Pid_bl.wei_pwm;
				Pid_fr.pwm = Pid_fr.pwm + Pid_fr.wei_pwm;
				Pid_br.pwm = Pid_br.pwm + Pid_br.wei_pwm;
			}
			pwm_time++;
			
			/*当PWM大于850时设为850，小于0时为0*/
			XIANFU(&Pid_fl);
			XIANFU(&Pid_fr);
			XIANFU(&Pid_bl);
			XIANFU(&Pid_br);
			
			/*
			t8pwm_1前左
			t8pwm_2后左
			t8pwm_3后右
			t8pwm_4前右
			*/
			if(Pid_fl.pwm>=0){CAR_FL=1;TIM_SetCompare1(TIM8,Pid_fl.pwm);}  //前左   
			else{CAR_FL=0;TIM_SetCompare1(TIM8,-Pid_fl.pwm);}
			if(Pid_fr.pwm>=0){CAR_FR=0;TIM_SetCompare4(TIM8,Pid_fr.pwm);}	 //前右		
			else{CAR_FR=1;TIM_SetCompare4(TIM8,-Pid_fr.pwm);}
			if(Pid_bl.pwm>=0){CAR_BL=1;TIM_SetCompare2(TIM8,Pid_bl.pwm);}  //后左  
			else{CAR_BL=0;TIM_SetCompare2(TIM8,-Pid_bl.pwm);}
			if(Pid_br.pwm>=0){CAR_BR=0;TIM_SetCompare3(TIM8,Pid_br.pwm);}	 //后右		
			else{CAR_BR=1;TIM_SetCompare3(TIM8,-Pid_br.pwm);}
			
		}	
		
//		sprintf(OLED_BUF,"%d",pwm_time);//显示
//			LCD_16_HanZi_ASCII(0,6,OLED_BUF);	
		/*双车时发送数据缓存地方*/
//		a_pwm[0]=qian_sum*10;
//		a_pwm[1]=hou_sum*10;
//		a_pwm[2]=chazhi_qh*10;
//		a_pwm[3]=Pid_br.sample*100;
//		a_pwm[4]=Pid_fr.pwm;
//		a_pwm[5]=Pid_fl.pwm;
//		SentDateTo_usart(a_pwm);//双车发送数据函数
		
		//pwm_time--;
//		printf("%d",pwm_time);
				
		TIM_ClearITPendingBit(TIM7,TIM_IT_Update);		
	}

}

void PWM_SET(void)      //开启定时器7并且进行一些PID参数设置
{
	//aa=0;
	tt=0;
	
	pwm_time = 0;
	
	stop_TIM = 1;
	
	Pid_fl.kp=KP;
	Pid_fr.kp=KP;
	Pid_bl.kp=KP;
	Pid_br.kp=KP;

	Pid_fl.ki=KI;
	Pid_fr.ki=KI;
	Pid_bl.ki=KI;
	Pid_br.ki=KI;

	Pid_fl.kd=KD;
	Pid_fr.kd=KD;
	Pid_bl.kd=KD;
	Pid_br.kd=KD;

	Pid_fl.pwm=0;
	Pid_fr.pwm=0;
	Pid_bl.pwm=0;
	Pid_br.pwm=0;

	Pid_fl.pre_det=0;
	Pid_fr.pre_det=0;
	Pid_bl.pre_det=0;
	Pid_br.pre_det=0;
	
	Encoder_Integral = 0;

	Pid_fl.last_det=0;
	Pid_fr.last_det=0;
	Pid_bl.last_det=0;
	Pid_br.last_det=0;
	
	tim7_start = 1;
	
	TIM7_BaseTime_Init(1500,720-1);		//100 	
//	fzopen();                         //防撞打开
}



//走t格，f 1前进0后退

void DJ_MOVE(u8 t,u8 f,u8 sp,u8 dj_1,u8 dj_2,u8 dj_3,u8 dj_4,u8 YS)		
{
//	printf("方向：%d，走：%d格，速度：%d，时间：",f,t,sp);
	u8 at = 0;		
	speed=1;	
	FX = f;	
	if(t == 1)//当需要走一格时
	{
		at = 0;
		speed=1;
		PWM_SET();			
		while(t > tt && stop_TIM)	       //等待车走过设定格数
		{		
			;
		}	
		if(f==1)
			delay_ms(10);
		else
			delay_ms(350);
	
		stop();	
	}	
	else if(t == 2)        //当需要走两格时
	{
		speed=2;
		PWM_SET();
		while(t-1 > tt && stop_TIM)	//最后一格前的路程用2档
		{
			;			
		}				
		
		speed=1;		
		while(t > tt && stop_TIM)	  //最后一格的路程用1档
		{		
			;
		}
		if(f==1)
			delay_ms(10);
		else
			delay_ms(350);
		stop();
	}
	else if(t >= 3)          //当需要走三格及以上时
	{
		speed = 1;	
		PWM_SET();
		while(tt < 1 && stop_TIM)	//第一格用1档速度
		{		
			;
		}
		
		speed = 3;		
		while(t-1 > tt && stop_TIM)	//中间用3速度
		{
			;
		}
	
		speed=1;
		while(t > tt && stop_TIM)	  //最后一格用1档速度
		{		
			;
		}
		if(f==1)
			delay_ms(10);
		else
			delay_ms(350);
		stop();
	}
}

u8 BMQ_MOVE(u8 f,u16 d,u8 mod)        //编码器行走函数
{
	speed=4;
	pidflag=0;
	if(f==1)//前进
	{
		FX=1;
		PWM_SET();
		sum_value1=0;
		sum_value2=0;
		sum_flag=1;
		
		if(mod==1)
		{
			while(1)
			{
				if((abs(sum_value1)>(d-4)*37)&&(abs(sum_value1)<(d+6)*37)) {stop();pidflag=1;return 1;}
				else if(abs(sum_value1)>(d+6)*37) {stop();pidflag=1;return 0;}//防止跑出
				
			}
		}
		else
			while(abs(sum_value1)<d*37||abs(sum_value2)<d*37);     //57是走1cm的值，修改这个值可以改变走1cm的距离
		sum_flag=0;

		stop();
	}
	
	if(f==0||f==9)//后退
	{
		FX=0;
		PWM_SET();
		sum_value1=0;
		sum_value2=0;
		sum_flag=1;

		if(mod==1)
		{
				while(1)
			{
				if((abs(sum_value1)>(d-4)*37)&&(abs(sum_value1)<(d+6)*37)) {stop();pidflag=1;return 1;}
				else if(abs(sum_value1)>(d+6)*37) {stop();pidflag=1;return 0;}//防止跑出
			}
		}
		else
			while(abs(sum_value1)<d*37||abs(sum_value2)<d*37);
		sum_flag=0;
	}
		stop();
		pidflag=1;
}

void DJ_MOVE_ZHUAN(u8 r,u8 f)		//r3左转4右转,f1前进0后退
{
//	
//	printf("转弯方向：%d,时间：",r);
	/*循迹延时转弯*/
//	speed=1;
//	if(r == 3)
//	{
//		FX = 2;			
//		PWM_SET();
//		delay_ms(300);
//		while(1)	
//		{
//			if((q_q_q>=2&&q_q_q<=4&&h_h_h<=4&&h_h_h>=2||pwm_time>=80|| stop_TIM == 0))//60
//			break;
//		}
//	}
//	if(r == 4)
//	{
//		FX = 3;			
//		PWM_SET();
//		delay_ms(300);
//		while(1)	
//		{
//			if((q_q_q>=2&&q_q_q<=4&&h_h_h<=4&&h_h_h>=2||pwm_time>=80||stop_TIM == 0))//60
//			break;
//		}
//	}
	/*编码器转弯*/
	
//	speed=3;
//	if(r == 3)
//	{
//		FX = 2;	
//		PWM_SET();
//		sum_value=0;
//		sum_flag=1;
//		while(abs(sum_value)<400);       //改这个值可以改变转弯的角度
//		
//		sum_flag=0;
//	}
//	if(r == 4)
//	{
//		FX = 3;	
//		PWM_SET();
//		sum_value=0;
//		sum_flag=1;
//		while(abs(sum_value)<420);
//		sum_flag=0;
//	}
//		
//      if(r == 6)    //左45
//	{
//		FX = 2;	
//		PWM_SET();
//		sum_value=0;
//		sum_flag=1;
//		while(abs(sum_value)<510);       //改这个值可以改变转弯的角度
//		
//		sum_flag=0;
//	}
//	if(r == 7)     //右45
//	{
//		FX = 3;	
//		PWM_SET();
//		sum_value=0;
//		sum_flag=1;
//		while(abs(sum_value)<425);
//		sum_flag=0;
//	}	
//	if(r == 8)    //左45
//	{
//		FX = 2;	
//		PWM_SET();
//		sum_value=0;
//		sum_flag=1;
//		while(abs(sum_value)<300);       //改这个值可以改变转弯的角度
//		
//		sum_flag=0;
//	}
//	if(r == 9)     //右45
//	{
//		FX = 3;	
//		PWM_SET();
//		sum_value=0;
//		sum_flag=1;
//		while(abs(sum_value)<200);
//		sum_flag=0;
//	}	

	/*六轴转弯*/

speed=5;
	if(r == 3)
	{

		float last_angle=0;
		angle_6 = 0;					   
		while (angle_6 == 0)						   // 等待数据
			;
		angle_6 = 0;	 				   // 使能串口1
		while (angle_6 == 0)						   // 等待数据
			;

		angle_cs+=90; 
		if(angle_cs>=360)
		angle_cs-=360;

		FX = 2;
		PWM_SET();		
		while ((angle_cs - angle_6 >= 3 || angle_cs - angle_6 <= -3)||((int16)(last_angle -angle_6)%360 > 10||(int16)(last_angle -angle_6)%360 <- 10)) // 当前值与目标值作差，差小于某个值则认为转好了
		{		
			// 	sprintf(OLED_BUF,"%.0f 	%5.0f",angle1,angle_6);//显示
			// LCD_16_HanZi_ASCII(0,6,OLED_BUF);	
			last_angle = angle_6;
		};
		sum_flag = 0;				
		angle_6 = 0;
	}
	if(r == 4)
	{
		float last_angle=0;			
		angle_6 = 0;
		while (angle_6 == 0)						   // 等待数据
			;
		angle_6 = 0;
		while (angle_6 == 0)						   // 等待数据
			;
		
		angle_cs-=90;
		if(angle_cs<=0)
		angle_cs+=360;

		FX = 3;
		PWM_SET();	
		while ((angle_cs - angle_6 >= 3 || angle_cs - angle_6 <= -3)||((int16)(last_angle -angle_6)%360 > 10||(int16)(last_angle -angle_6)%360 <- 10))
			{			
			// 	sprintf(OLED_BUF,"%.0f 	%5.0f",angle1,angle_6);//显示
			// LCD_16_HanZi_ASCII(0,6,OLED_BUF);	
				last_angle = angle_6;
			};
		sum_flag = 0;				
		angle_6 = 0;
	}
	
			
		if(r == 5)//右转30度
	{
		float last_angle=0;			
		USART_Cmd(USART1, ENABLE);					   // 使能串口1
		angle_6 = 0;
		while (angle_6 == 0)						   // 等待数据
			;
		angle_6 = 0;
		while (angle_6 == 0)						   // 等待数据
			;
		
		angle_cs-=30;
		if(angle_cs<=0)
		angle_cs+=360;

		FX = 3;
		PWM_SET();	
		while ((angle_cs - angle_6 >= 3 || angle_cs - angle_6 <= -3)||((int16)(last_angle -angle_6)%360 > 10||(int16)(last_angle -angle_6)%360 <- 10))
			{			
			// 	sprintf(OLED_BUF,"%.0f 	%5.0f",angle1,angle_6);//显示
			// LCD_16_HanZi_ASCII(0,6,OLED_BUF);	
				last_angle = angle_6;
			};
		USART_Cmd(USART1, DISABLE);					   // 使能串口1
		sum_flag = 0;				
		angle_6 = 0;
	}
		if(r == 6)   //左转45度
	{
		float last_angle=0;
		angle_6 = 0;					   // 使能串口1
		while (angle_6 == 0)						   // 等待数据
			;
		angle_6 = 0;	 				   // 使能串口1
		while (angle_6 == 0)						   // 等待数据
			;

		angle_cs+=43; 
		if(angle_cs>=360)
		angle_cs-=360;

		FX = 2;
		PWM_SET();		
		while ((angle_cs - angle_6 >= 3 || angle_cs - angle_6 <= -3)||((int16)(last_angle -angle_6)%360 > 8||(int16)(last_angle -angle_6)%360 <- 8)) // 当前值与目标值作差，差小于某个值则认为转好了
		{		
			// 	sprintf(OLED_BUF,"%.0f 	%5.0f",angle1,angle_6);//显示
			// LCD_16_HanZi_ASCII(0,6,OLED_BUF);	
			last_angle = angle_6;
		};
		sum_flag = 0;				
		angle_6 = 0;
	}
		if(r == 7)		//右转45度
	{
		float last_angle=0;			
		angle_6 = 0;
		while (angle_6 == 0)						   // 等待数据
			;
		angle_6 = 0;
		while (angle_6 == 0)						   // 等待数据
			;
		
		angle_cs-=45;
		if(angle_cs<=0)
		angle_cs+=360;

		FX = 3;
		PWM_SET();	
		while ((angle_cs - angle_6 >= 3 || angle_cs - angle_6 <= -3)||((int16)(last_angle -angle_6)%360 > 10||(int16)(last_angle -angle_6)%360 <- 10))
			{			
			// 	sprintf(OLED_BUF,"%.0f 	%5.0f",angle1,angle_6);//显示
			// LCD_16_HanZi_ASCII(0,6,OLED_BUF);	
				last_angle = angle_6;
			};
		sum_flag = 0;				
		angle_6 = 0;
	}

	stop();
	turn_af=1;
}


void DJ_MOVE_YS(u8 t,u8 f,u8 sp,u16 tim)	//延时停车
{
	speed=4;
	FX = f;
	PWM_SET();		
	if(t > 1)
	{
		while(t-1 > tt)	
		{		
			;
		}
	}	
	speed=4;
	pwm_time = 0;	
	while(pwm_time<tim)			//速度1，一格67
	{		
		;
	}	
	stop();

}

void DJ_MOVE_YS_XC(u8 t,u8 f,u8 sp,u16 tim)	//延时下车停车
{
	int i;
	speed=4;
	FX = f;
	PWM_SET();		
//	if(t > 1)
//	{
//		while(t-1 > tt)	
//		{		
//			;
//		}
//	}	
	speed=4;
	pwm_time = 0;	
//	while(i<tim)			//速度1，一格67
//	{		
//		i++;
//		delay_ms(100);
//	}
	for(i=0;i<tim;i++)		//速度1，一格67
	{		
		delay_ms(150);
	}
//	delay_ms(9000);
	stop();

}





