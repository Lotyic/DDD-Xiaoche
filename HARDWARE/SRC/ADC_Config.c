/*
  ******************************************************************************
  * @??            ???(668484)
  * @STM32???     V3.5.0
  * @??????    2016-6-16
  * @??            ?DMA?AD??,???????IO_config.h
  * @??            6?????????????ADC_DMA_ConvertedValue[6]?,
  *                  ?????
  ******************************************************************************
*/

#include "ADC_Config.h"

#include "oled.h"
#include "usart.h"	

u8  Key_Num;
u8 Keykey_Flag;      //???????
u8 KEY_TYPE;

vu16 ADC_DMA_ConvertedValue[6];	//?DMA?6??AD???

void ADC_DMAInit(void)
{
	DMA_InitTypeDef DMA_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//??DMA??

	DMA_DeInit(DMA1_Channel1); //? DMA???1?????????
	DMA_InitStruct.DMA_PeripheralBaseAddr =(u32)&ADC1->DR;//DMA???????	
	DMA_InitStruct.DMA_MemoryBaseAddr = (vu32)ADC_DMA_ConvertedValue;//????????
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;		//???????????
	DMA_InitStruct.DMA_BufferSize = 1;	//DMA????
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//?????????
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;//?????????
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//??????? 16 ?
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //??????? 16 ?
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;//?????????
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;//DMA?? x?????? 
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;//DMA?? x????????????
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);

	DMA_Cmd(DMA1_Channel1, ENABLE);
}
void ADC_DMA_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	ADC_DMAInit();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE);	  
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  //??ADC????6 72M/6=12 

	GPIO_InitStructure.GPIO_Pin = ADC_DMA_IO0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_DMA_PORT,&GPIO_InitStructure);


	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 		//  ADC???????
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;   			//?????
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	 	//???????????
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//??????????????
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC ?????
	ADC_InitStructure.ADC_NbrOfChannel = 1;	 				//????????? ADC ?????
	ADC_Init(ADC1,&ADC_InitStructure);


	ADC_RegularChannelConfig(ADC1, ADC_DMA_CH0, 1, ADC_SampleTime_55Cycles5);

	ADC_DMACmd(ADC1, ENABLE); //??ADC1?DMA
	ADC_Cmd(ADC1, ENABLE);//??ADC1

	ADC_ResetCalibration(ADC1);//????
	while(ADC_GetResetCalibrationStatus(ADC1));  //????????	
	ADC_StartCalibration(ADC1);	  //????
	while(ADC_GetCalibrationStatus(ADC1));  //??????

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//??ADC??
}

void ADKey_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF |RCC_APB2Periph_ADC3, ENABLE );	  //??ADC3????

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //??ADC????6 72M/6=12,ADC????????14M

	//PC0 ??????????
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//??????
	GPIO_Init(GPIOF, &GPIO_InitStructure);	

	ADC_DeInit(ADC3);  //??ADC3

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC????:ADC1?ADC3???????
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//????????????
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//?????????????
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//??????????????
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC?????
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//?????????ADC?????
	
	ADC_Init(ADC3, &ADC_InitStructure);	//??ADC_InitStruct???????????ADCx????   

	ADC_Cmd(ADC3, ENABLE);	//?????ADC3
	
	ADC_ResetCalibration(ADC3);	//??????
	while(ADC_GetResetCalibrationStatus(ADC3));	//????????
	ADC_StartCalibration(ADC3);	 //??AD??
	while(ADC_GetCalibrationStatus(ADC3));	 //??????
}

u16 Get_Key_Adc(u8 ch)
{
	ADC_RegularChannelConfig(ADC3, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC3,ADC??,?????239.5??
  
	ADC_SoftwareStartConvCmd(ADC3, ENABLE);		//?????ADC3?????????
	 
	while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC ));//??????

	return ADC_GetConversionValue(ADC3);	//??????ADC3????????
}

u16 Get_Key_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val = 0;
	u8 t;
	for(t = 0;t < times;t++)
	{
		temp_val += Get_Key_Adc(ch);
	}
	return temp_val / times;
}

u8 ADKey_Deal(u16 ADKey_Value)
{
//	
	// sprintf(OLED_BUF,"%d   ",ADKey_Value/10);
	// LCD_16_HanZi_ASCII(0,0,OLED_BUF);
	
//	u16 adcx;

//	float temp;
//	

//	
//	adcx=ADC_DMA_ConvertedValue[0];    //??PC0-PC2?????12?AD?
//	temp=(float)adcx*(3.3/4096);		//??????
//			
//	if(temp>0.1  && temp<0.50)			return 's';
//	else if(temp>2.80 && temp<3.30)		return 'x';
//	else if(temp>1.40 && temp<1.80)		return 'z';
//	else if(temp>2.00 && temp<2.40)		return 'y';
//	else if(temp>0.90 && temp<1.25)		return 'o';
	
	
	
//	ADKey_Value = ADKey_Value/10;
//	
//	if(ADKey_Value >= 340 && ADKey_Value <= 360)		return '1';
//	else if(ADKey_Value >= 300 && ADKey_Value <= 320)	return '4';
//	else if(ADKey_Value >= 265 && ADKey_Value <= 285)	return '7';
//	else if(ADKey_Value >= 240 && ADKey_Value <= 255)	return '8';
//	else if(ADKey_Value >= 210 && ADKey_Value <= 230)	return '5';
//	else if(ADKey_Value >= 188 && ADKey_Value <= 205)	return '2';
//	else if(ADKey_Value >= 168 && ADKey_Value <= 185)	return '3';
//	else if(ADKey_Value >= 150 && ADKey_Value <= 165)	return '6';
//	else if(ADKey_Value >= 135 && ADKey_Value <= 149)	return '9';
//	else if(ADKey_Value >= 120 && ADKey_Value <= 133)	return '0';
//	else if(ADKey_Value >= 105 && ADKey_Value <= 118)	return 's';
//	else if(ADKey_Value >= 92  && ADKey_Value <= 104)	return 'o';
//	else if(ADKey_Value >= 78  && ADKey_Value <= 90 )	return 'z';
//	else if(ADKey_Value >= 65  && ADKey_Value <= 75 )	return 'x';
//	else if(ADKey_Value >= 45  && ADKey_Value <= 62 )	return 'y';
//	else if(ADKey_Value >= 30  && ADKey_Value <= 42 )	return '#';
//	else if(ADKey_Value >= 15  && ADKey_Value <= 28 )	return '*';
//	else												return 0;
	
	// if(ADKey_Value > 1900 && ADKey_Value < 2000)return '*';   //.
	// else if(ADKey_Value > 650 && ADKey_Value < 750)return '#';	   //#
	// else if(ADKey_Value > 2100 && ADKey_Value < 2200)return 'y';   //?
	// else if(ADKey_Value > 3000 && ADKey_Value < 3200)return 'x';   //?
	// else if(ADKey_Value > 3500 && ADKey_Value < 3600)return 'z';   //?
	// else if(ADKey_Value > 2700 && ADKey_Value < 2800)return 'o';  //?
	// else if(ADKey_Value > 2400 && ADKey_Value < 2500)return 's'; //?
	// else if(ADKey_Value > 800 && ADKey_Value < 900)return '0';
	// else if(ADKey_Value > 500 && ADKey_Value < 600)return '9';
	// else if(ADKey_Value > 300 && ADKey_Value < 400)return '6';
	// else if(ADKey_Value > 150 && ADKey_Value < 250)return '3';
	// else if(ADKey_Value > 1200 && ADKey_Value < 1300)return '2';
	// else if(ADKey_Value > 1100 && ADKey_Value < 1150)return '5';
	// else if(ADKey_Value > 910 && ADKey_Value < 1000)return '8';
	// else if(ADKey_Value > 1700 && ADKey_Value < 1800)return '7';
	// else if(ADKey_Value > 1500 && ADKey_Value < 1600)return '4';
	// else if(ADKey_Value > 1400 && ADKey_Value < 1499)return '1';
	// else return 0;


if(ADKey_Value > 2390 && ADKey_Value < 2490)return '*';   //.
	else if(ADKey_Value > 2810 && ADKey_Value < 2910)return '#';	   //#
	else if(ADKey_Value > 3640 && ADKey_Value < 3740)return 'y';   //?
	else if(ADKey_Value > 3230 && ADKey_Value < 3330)return 'x';   //?
	else if(ADKey_Value > 3420 && ADKey_Value < 3520)return 'z';   //?
	else if(ADKey_Value > 3840 && ADKey_Value < 3940)return 'o';  //?
	else if(ADKey_Value > 3040 && ADKey_Value < 3140)return 's'; //?
	else if(ADKey_Value > 2590 && ADKey_Value < 2690)return '0';
	else if(ADKey_Value > 2190 && ADKey_Value < 2290)return '9';
	else if(ADKey_Value > 1580 && ADKey_Value < 1680)return '6';
	else if(ADKey_Value > 970 && ADKey_Value < 1070)return '3';
	else if(ADKey_Value > 760 && ADKey_Value < 860)return '2';
	else if(ADKey_Value > 1410 && ADKey_Value < 1510)return '5';
	else if(ADKey_Value > 1990 && ADKey_Value < 2090)return '8';
	else if(ADKey_Value > 1810 && ADKey_Value < 1910)return '7';
	else if(ADKey_Value > 1180 && ADKey_Value < 1280)return '4';
	else if(ADKey_Value > 550 && ADKey_Value < 650)return '1';
	else return 0;



			
		
}
u8 DMA_AD(void)
{
	u16 adcx;

	float temp;
	
	// sprintf(OLED_BUF,"%.1f     ",temp);
	// LCD_16_HanZi_ASCII(0,0,OLED_BUF);	
	
	adcx=ADC_DMA_ConvertedValue[0];    //??PC0-PC2?????12?AD?
	temp=(float)adcx*(3.3/4096);		//??????
			
	if(temp>0.1  && temp<0.50)			return 's';
	else if(temp>2.80 && temp<3.30)		return 'x';
	else if(temp>1.40 && temp<1.80)		return 'z';
	else if(temp>2.00 && temp<2.40)		return 'y';
	else if(temp>0.90 && temp<1.25)		return 'o';
	else 	return 0;
}


void AD_AnJian(void)
{	
	u8 AD_Get = 0;
	
	static uint8 ADKey_State=0;//0??????,1??????
	static uint16 AD_Time=0,Long_count=0,Long_flag=0; 
	
	AD_Get = ADKey_Deal(Get_Key_Adc_Average(ADC_Channel_4,1));
//		sprintf(OLED_BUF,"%c     ",AD_Get);
//	LCD_16_HanZi_ASCII(0,0,OLED_BUF);	
	if(AD_Get)
	{
		Long_count++;
		if(Long_count>=7500)//??????
		{
			Long_count=0;//?????????
			Long_flag = 1;
			Keykey_Flag=1;
			Key_Num=AD_Get;
			KEY_TYPE=AD_Get;
			
//			BUZZ_YIN(3370);
		}

	}
	else 
	{
		Long_count = 0;
		Long_flag  = 0;
	}

	if(Long_flag == 0)
	{
		if(AD_Get>0&&ADKey_State==0)    //??????
		{
			AD_Time++;                  //??
			if(AD_Time>=20)
			{ 
				AD_Time=0;

				ADKey_State=1;            //??????
				Keykey_Flag=1;

				Key_Num=AD_Get;
				KEY_TYPE=AD_Get;
//				BUZZ_YIN(3370);
			}
		}

		if(ADKey_State&&AD_Get==0)//??????
		{

			ADKey_State=0;

		}
	}
//	if(SHAUNG_UI == 1)//??????
//	{
//		Keykey_Flag=1;
//		KEY_TYPE='#';
//	}
}




