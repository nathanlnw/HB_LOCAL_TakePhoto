/*
     Device_808.C       ºÍ808   Ð­ÒéÏà¹ØµÄ I/O ¹Ü½ÅÅäÖÃ     
*/

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"


//-----  WachDog related----
u8    wdg_reset_flag=0;    //  Task Idle Hook Ïà¹Ø

void WatchDog_Feed(void)
{
    if(wdg_reset_flag==0)
           IWDG_ReloadCounter();   
}

void  reset(void)
{
   //IWDG_SetReload(0);
  // IWDG->KR = 0x00001;  //not regular
  wdg_reset_flag=1; 
} 
FINSH_FUNCTION_EXPORT(reset, ststem reset);
void WatchDogInit(void)
{    
  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: LSI/32 */
    /*   prescaler            min/ms    max/ms
         4                        0.1             409.6
         8                        0.2             819.2
         16                      0.4             1638.4
         32                      0.8              3276.8
         64                      1.6              6553.5
         128                    3.2              13107.2
         256                    6.4              26214.4   
  */
  IWDG_SetPrescaler(IWDG_Prescaler_16);

  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(0X4AAA);//(LsiFreq/128);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

void  APP_IOpinInit(void)   //³õÊ¼»¯ ºÍ¹¦ÄÜÏà¹ØµÄIO ¹Ü½Å
{
  	GPIO_InitTypeDef        gpio_init;

     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);      
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	 

    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz; 
    gpio_init.GPIO_OType = GPIO_OType_PP;  
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL; 	 
 // 		IN
	//------------------- PE8 -----------------------------
	gpio_init.GPIO_Pin	 = GPIO_Pin_8;	  //½ô¼±±¨¾¯
	gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
	GPIO_Init(GPIOE, &gpio_init);
	//------------------- PE9 -----------------------------
	gpio_init.GPIO_Pin	 = GPIO_Pin_9;				//------ACC  ×´Ì¬
	gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
	GPIO_Init(GPIOE, &gpio_init);
	//------------------- PE7 -----------------------------
	gpio_init.GPIO_Pin	 = GPIO_Pin_7;				//------³µÃÅ¿ª¹Ø×´Ì¬  0 ÓÐÐ§  ³£Ì¬ÏÂÎª¸ß   
	gpio_init.GPIO_Mode  = GPIO_Mode_IN;   //Èç¹ûÖ»½ÓÉ²³µ£¬ÄÇ¾ÍÓÃPE5µ±É²³µ¼àÊÓ 
	GPIO_Init(GPIOE, &gpio_init); 
 
   //	OUT
   
   //------------------- PB1 -----------------------------
   gpio_init.GPIO_Pin	= GPIO_Pin_1;   //------Î´¶¨Òå   Êä³ö ³£Ì¬ÖÃ0  
   gpio_init.GPIO_Mode	= GPIO_Mode_OUT; 
   GPIO_Init(GPIOB, &gpio_init); 
   
   gpio_init.GPIO_Pin	= GPIO_Pin_6;   //------Êä³ö ³£Ì¬ÖÃ0   PC13  ·äÃùÆ÷
   gpio_init.GPIO_Mode	= GPIO_Mode_OUT;  
   GPIO_Init(GPIOB, &gpio_init);  
   GPIO_ResetBits(GPIOB,GPIO_Pin_6);  // ¹Ø±Õ·äÃùÆ÷	   	 
   
   
 //==================================================================== 
 //-----------------------Ð´¼ÌµçÆ÷³£Ì¬ÏÂµÄÇé¿ö------------------
 //-----------------------Ð´¼ÌµçÆ÷³£Ì¬ÏÂµÄÇé¿ö------------------
  if(JT808Conf_struct.relay_flag==1)
  	GPIO_SetBits(GPIOB,GPIO_Pin_1);	 //¼ÌµçÆ÷¶Ï¿ª 
  else
       GPIO_ResetBits(GPIOB,GPIO_Pin_1);	 //¼ÌµçÆ÷±ÕºÏ
 //GPIO_SetBits(GPIOB,GPIO_Pin_1);	 //Êä³ö³£Ì¬ ÖÃ 0     

// GPIO_ResetBits(GPIOA,GPIO_Pin_13);	 // ¹Ø±Õ·äÃùÆ÷          
 /*
      J1 ½Ó¿Ú ³õÊ¼»¯
 */
	 //---------- PA0--------------------- 
  // gpio_init.GPIO_Pin  =GPIO_Pin_0; 				//-----  PIN 1   ËÙ¶È´«¸ÐÆ÷   // ÔÝÊ±ÎÞÓÃ
  // gpio_init.GPIO_Mode = GPIO_Mode_IN;
   //GPIO_Init(GPIOA, &gpio_init);
   //------------- -- --------------
   //gpio_init.GPIO_Pin	 = GPIO_Pin_6;				//------PIN  2   NULL 
  // gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   //GPIO_Init(GPIOE, &gpio_init);
    //--------------- --------------
   //gpio_init.GPIO_Pin	 = GPIO_Pin_9;				//------PIN 3    NULL 
   //gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   //GPIO_Init(GPIOD, &gpio_init);
    //------------- PC0 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_0;				//------PIN 4    Ô¶¹âµÆ
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOC, &gpio_init); 
    //------------- PC1----------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_1;				//------PIN 5    ½ü¹âµÆ
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOC, &gpio_init); 
    //------------- PA1 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_1;				//------PIN 6   ³µÃÅ
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOA, &gpio_init);
    //------------- PC3 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_3;				//------PIN 7   ÎíµÆ (AD1)
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOC, &gpio_init);
    //------------- PC2 --------------   
   gpio_init.GPIO_Pin	 = GPIO_Pin_2;				//------PIN 8   ÓÒ×ªµÆ(AD2)    
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOC, &gpio_init);  
         
    //------------- PE11 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_11;				//------PIN 9   É²³µµÆ
   gpio_init.GPIO_Mode  = GPIO_Mode_IN; 
   GPIO_Init(GPIOE, &gpio_init);
    //------------- PE10 --------------
   gpio_init.GPIO_Pin	 = GPIO_Pin_10;				//------PIN 10  ×ó×ªµÆ
   gpio_init.GPIO_Mode  = GPIO_Mode_IN;  
   GPIO_Init(GPIOE, &gpio_init);  
 
   //-----------------------------------------------------------------
   

   //------- ËÙ¶ÈÐÅºÅÏß ---------------
    gpio_init.GPIO_Pin = GPIO_Pin_0; 
    gpio_init.GPIO_Mode = GPIO_Mode_AF;
    gpio_init.GPIO_Speed = GPIO_Speed_100MHz; 
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL; 
    GPIO_Init(GPIOA, &gpio_init); 
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2); 


}





/*    
     -----------------------------
    1.    ÊäÈë¹Ü½Å×´Ì¬¼à²â
     ----------------------------- 
*/
u8  ACC_StatusGet(void)
{  
    // ACC ×´Ì¬Òý½Å   
    //·µ»Ø  ·Ç0 Îª  ACC ¿ª   
    //·µ»Ø  0  Îª   ACC ¹Ø
   return(!GPIO_ReadInputDataBit(ACC_IO_Group,ACC_Group_NUM)); 
}

u8  WARN_StatusGet(void)
{
    // ½ô¼±±¨¾¯ ×´Ì¬Òý½Å   
    //·µ»Ø  ·Ç0 Îª  ±¨¾¯°´Å¥°´ÏÂ  
    //·µ»Ø  0  Îª   ±¨¾¯°´Å¥¶Ï¿ª
   return (!GPIO_ReadInputDataBit(WARN_IO_Group,WARN_Group_NUM));   
}
u8  MainPower_cut(void)
{ 
    // ¶Ïµç±¨¾¯ ×´Ì¬Òý½Å   
    //·µ»Ø  ·Ç0 Îª  Ö÷µç¶Ï¿ª  
    //·µ»Ø  0  Îª   Ö÷µç¹¤×÷ 
 // return (GPIO_ReadInputDataBit(POWER_IO_Group,POWER_Group_NUM)&0x01) ; 
   return false;
}


//-------------------------------------------------------------------------------------------------
u8  BreakLight_StatusGet(void)
{ 
	//	--------------J1pin8	PE11	         É²³µµÆ------> ×Ø
	   return(!GPIO_ReadInputDataBit(BREAK_IO_Group,BREAK_Group_NUM));	//PE11
			//		 ½Ó¸ß  ´¥·¢
}
u8  LeftLight_StatusGet(void)
{
  //  --------------J1pin10	   PE10		   ×ó×ªµÆ------>ºì
	   return (!GPIO_ReadInputDataBit(LEFTLIGHT_IO_Group,LEFTLIGHT_Group_NUM));	//	PE10
			//		 ½Ó¸ß  ´¥·¢
}	
u8  RightLight_StatusGet(void)
{
//	--------------J1pin8   PC2			  ÓÒ×ªµÆ------>°×
		 return(!GPIO_ReadInputDataBit(RIGHTLIGHT_IO_Group,RIGHTLIGHT_Group_NUM));  //PC2 
		     //	   ½Ó¸ß  ´¥·¢ 
}			

u8  FarLight_StatusGet(void)
{
  //  --------------J1pin4		PC0	           Ô¶¹âµÆ-----> ºÚ
	   return (!GPIO_ReadInputDataBit(FARLIGHT_IO_Group,FARLIGHT_Group_NUM));	// PC0
	//		 ½Ó¸ß  ´¥·
rt_kprintf("¼ì²âµ½Òý½Å¸ß----Ô¶¹â");
}	
u8  NEARLight_StatusGet(void)
{
  //  --------------J1pin5		PC1	          ½ü¹âµÆ------>  »Æ
	   return (!GPIO_ReadInputDataBit(NEARLIGHT_IO_Group,NEARLIGHT_Group_NUM));	// PC1
	//		 ½Ó¸ß  ´¥·¢
}

u8 FOGLight_StatusGet(void)
{
// --------------J1pin7    PC3			    ÎíµÆ     ------>   ÂÌ
		return(!GPIO_ReadInputDataBit(FOGLIGHT_IO_Group,FOGLIGHT_Group_NUM));  //PC3 
			//		 ½Ó¸ß  ´¥·¢
}	
u8  DoorLight_StatusGet(void)
{
 //  --------------J1pin6	   PA1		   ³µÃÅµÆ    ------>»Ò
	   return (!GPIO_ReadInputDataBit(DOORLIGHT_IO_Group,DOORLIGHT_Group_NUM));	// PA1
			//		 ½Ó¸ß  ´¥·¢
}		


/*    
     -----------------------------
    2.  ¿ØÖÆÊä³ö
     ----------------------------- 
*/
void  Enable_Relay(void)
{  // ¶Ï¿ª¼ÌµçÆ÷
 
 GPIO_SetBits(RELAY_IO_Group,RELAY_Group_NUM); // ¶Ï¿ª	
}
void  Disable_Relay(void)
{ // ½ÓÍ¨¼ÌµçÆ÷
   
   GPIO_ResetBits(RELAY_IO_Group,RELAY_Group_NUM); // Í¨	 
}


u8  Get_SensorStatus(void)   
{        // ²éÑ¯´«¸ÐÆ÷×´Ì¬
   u8  Sensorstatus=0;
   /*  
     -------------------------------------------------------------
              F4  ÐÐ³µ¼ÇÂ¼ÒÇ TW703   ¹Ü½Å¶¨Òå
     -------------------------------------------------------------
     ×ñÑ­  GB10956 (2012)  Page26  ±íA.12  ¹æ¶¨
    -------------------------------------------------------------
    | Bit  |      Note       |  ±Ø±¸|   MCUpin  |   PCB pin  |   Colour | ADC
    ------------------------------------------------------------
        D7      É²³µ           *            PE11             9                ×Ø
        D6      ×ó×ªµÆ     *             PE10            10               ºì
        D5      ÓÒ×ªµÆ     *             PC2              8                °×
        D4      Ô¶¹âµÆ     *             PC0              4                ºÚ
        D3      ½ü¹âµÆ     *             PC1              5                »Æ
        D2      ÎíµÆ          add          PC3              7                ÂÌ      *
        D1      ³µÃÅ          add          PA1              6                »Ò      *
        D0      Ô¤Áô
   */

   //  1.   D7      É²³µ           *            PE11             J1 pin9                ×Ø
		  if(BreakLight_StatusGet())  //PE11  
		   {   //		½Ó¸ß  ´¥·¢
			   Sensorstatus|=0x80;
			  BD_EXT.FJ_IO_1 |=0x80;  //  bit7 
			  BD_EXT.Extent_IO_status |= 0x10;  // bit4 ---->É²³µ
			   // rt_kprintf("\r\n ÎÒÊÇ1"); 
		   }
		  else
		   {  //   ³£Ì¬
			   Sensorstatus&=~0x80;		
			    BD_EXT.FJ_IO_1 &=~0x80;  //  bit7 
			    BD_EXT.Extent_IO_status &= ~0x10; //bit4 ---->É²³µ
		   } 
	// 2.  D6      ×ó×ªµÆ     *             PE10            J1 pin10              ºì
		 if(LeftLight_StatusGet())	//	PE10 
		  {   //	   ½Ó¸ß  ´¥·¢
			  Sensorstatus|=0x40;
			   BD_EXT.FJ_IO_1 |=0x40;  //  bit6
			   BD_EXT.Extent_IO_status |= 0x08;//bit3---->  ×ó×ªµÆ
		  }
		 else
		  {  //   ³£Ì¬
			 Sensorstatus&=~0x40; 	
			 BD_EXT.FJ_IO_1 &=~0x40;  //  bit6 
			 BD_EXT.Extent_IO_status &= ~0x08; //bit3---->  ×ó×ªµÆ
		  }
   //  3.  D5      ÓÒ×ªµÆ     *             PC2             J1  pin8                °×
	     if(RightLight_StatusGet())	//PC2 
	     { //		 ½Ó¸ß  ´¥·¢
				Sensorstatus|=0x20;
				 BD_EXT.FJ_IO_1 |=0x20; //bit5
				 BD_EXT.Extent_IO_status |= 0x04;// bit2----> ÓÒ×ªµÆ
		}
            else
		{  //	³£Ì¬
			   Sensorstatus&=~0x20;		
		   BD_EXT.FJ_IO_1 &=~0x20; //bit5
		   BD_EXT.Extent_IO_status &= ~0x04;//bit2----> ÓÒ×ªµÆ
		} 
   
   // 4.  D4      Ô¶¹âµÆ     *             PC0              J1 pin4                ºÚ
	   if(FarLight_StatusGet())	// PC0 
		{	//		 ½Ó¸ß  ´¥·¢
			Sensorstatus|=0x10;
			BD_EXT.Extent_IO_status |= 0x02; //bit 1  ----->  Ô¶¹âµÆ
                   
		}
	   else
		{  //	³£Ì¬
		   Sensorstatus&=~0x10;		
		   BD_EXT.Extent_IO_status&= ~0x02;//bit 1  ----->  Ô¶¹âµÆ

		}  
    //5.   D3      ½ü¹âµÆ     *             PC1              J1 pin5                »Æ
   		 if(NEARLight_StatusGet())  // PC1
		  {   //       ½Ó¸ß  ´¥·¢
		      Sensorstatus|=0x08;
		       BD_EXT.FJ_IO_1 |=0x10; //bit4	  
		       BD_EXT.Extent_IO_status |= 0x01; //bit 0  ----->  ½ü¹âµÆ
				 //rt_kprintf("\r\n ÎÒÊÇ5"); 
		  }
		 else
		  {  //	  ³£Ì¬
			 Sensorstatus&=~0x08;		
			  BD_EXT.FJ_IO_1 &=~0x10; //bit4
			   BD_EXT.Extent_IO_status &=~0x01; //bit 0  ----->  ½ü¹âµÆ 
			  
		  } 
  //  6.    D2      ÎíµÆ          add          PC3              7                ÂÌ      *
          if(FOGLight_StatusGet())  //PC3  
		  {   //	   ½Ó¸ß  ´¥·¢
			  Sensorstatus|=0x04;
			  BD_EXT.FJ_IO_1 |=0x08; //bit3	
			  BD_EXT.Extent_IO_status |= 0x40;//  bit6 ----> ÎíµÆ
			 //  rt_kprintf("\r\n ÎÒÊÇ6"); 
		  }
		 else
		  {  //   ³£Ì¬
			  Sensorstatus&=~0x04;
			  BD_EXT.FJ_IO_1 &=~0x08; //bit3
			  BD_EXT.Extent_IO_status &= ~0x40;//  bit6 ----> ÎíµÆ
		  } 
  // 7.    D1      ³µÃÅ          add          PA1              6                »Ò      *
	    if(DoorLight_StatusGet())  // PE3     
		{	//		 ½Ó¸ß  ´¥·¢
			Sensorstatus|=0x02;
			 BD_EXT.FJ_IO_2 |=0x01; //bit2       
		}
	   else
		{  //	³£Ì¬
		       Sensorstatus&=~0x02;		
		       BD_EXT.FJ_IO_2 |=0x01; //bit2 
		}	
 			   
 //    8.  Reserved

   return Sensorstatus;
} 

void  IO_statusCheck(void)
{
      Vehicle_sensor=Get_SensorStatus();
	/*	Sensor_buf[save_sensorCounter].DOUBTspeed=GPS_speed/10;     //   ËÙ¶È  µ¥Î»ÊÇkm/h ËùÒÔ³ýÒÔ10
        Sensor_buf[save_sensorCounter++].DOUBTstatus=Vehicle_sensor;//   ×´Ì¬ 
		if(save_sensorCounter>100) 
			{
                       save_sensorCounter=0; 
			  sensor_writeOverFlag=1; 
			}  
        */ 
}

void  ACC_status_Check(void)
{
                 //------------³µÁ¾ÔËÐÐ×´Ì¬Ö¸Ê¾ ---------------   
			    if(ACC_StatusGet())           //bit 0
				{	
				   Vehicle_RunStatus|=0x01;
		           //   ACC ON		 ´ò»ð 
				   StatusReg_ACC_ON();  // ACC  ×´Ì¬¼Ä´æÆ÷   
				   Sleep_Mode_ConfigExit(); // ÐÝÃßÏà¹Ø
			    }
				else
				{
				   Vehicle_RunStatus&=~0x01;
				   //	  ACC OFF	   ¹Ø»ð
				   StatusReg_ACC_OFF();  // ACC  ×´Ì¬¼Ä´æÆ÷ 		  
				   Sleep_Mode_ConfigEnter(); // ÐÝÃßÏà¹Ø
				}  
}

/*    
     -----------------------------
    2.  Ó¦ÓÃÏà¹Ø
     ----------------------------- 
*/
 void TIM2_Configuration(void) //Ö»ÓÃÒ»¸öÍâ²¿Âö³å¶Ë¿Ú
 {
    TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure; 	

    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ÅäÖÃRCC    

    //ÅäÖÃTIMER2×÷Îª¼ÆÊýÆ÷
    TIM_DeInit(TIM2);

    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler=0x0000;   //Ô¤·ÖÆµ71£¬¼´72·ÖÆµ£¬µÃ1M   
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;  
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  

	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); // Time base configuration	
	TIM_ETRClockMode2Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0x0F);
	TIM_SetCounter(TIM2, 0); 
	TIM_Cmd(TIM2, ENABLE);      

  
}

void Init_ADC(void)
{
  
  ADC_InitTypeDef   ADC_InitStructure;
  GPIO_InitTypeDef		gpio_init;
ADC_CommonInitTypeDef  ADC_CommonInitStructure;

 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
 

//------Configure PC.5 (ADC Channel15) as analog input -------------------------
gpio_init.GPIO_Pin = GPIO_Pin_5;
gpio_init.GPIO_Mode = GPIO_Mode_AIN;
GPIO_Init(GPIOC, &gpio_init);


  /* ADC Common configuration *************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; /*ÔÚ¶ÀÁ¢Ä£Ê½ÏÂ Ã¿¸öADC½Ó¿Ú¶ÀÁ¢¹¤×÷*/
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);


 /* ADC1 regular channel7 configuration *************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_3Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

  /* Enable ADC3 DMA */
//  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC3 */
  ADC_Cmd(ADC1, ENABLE);

  ADC_SoftwareStartConv(ADC1);

}




//==========================================================
void  sys_status(void)
{
     rt_kprintf("\r\n ×´Ì¬²éÑ¯: "); 	     
     //-----  ±¨¾¯  ---	 
     if(WARN_StatusGet())
	                 rt_kprintf(" ½ô¼±±¨¾¯      "); 	      
    //     ACC 	 
     if(ACC_StatusGet())
                        rt_kprintf("ACC ¿ª    "); 
     else
	 	          rt_kprintf("ACC ¹Ø    ");  
    //     AD µçÑ¹ 
    rt_kprintf ("\r\n  »ñÈ¡µ½µÄµç³ØADÊýÖµÎª:	%d	",ADC_ConvertedValue);	  
    rt_kprintf(" AD µçÑ¹: %d.%d  V  ",AD_Volte/10,AD_Volte%10);    	        
    //    ÐÅºÅÏß	 
     rt_kprintf("    %s      ",XinhaoStatus);       
    //   ¶¨Î»Ä£Ê½	
     rt_kprintf("\r\n ¶¨Î»Ä£Ê½:    ");  
		      switch(GpsStatus.Position_Moule_Status)
		      	{  
		      	         case 1:           rt_kprintf(" BD");  
						 	break;
				  case 2:           rt_kprintf(" GPS ");  
				  	              break;
				  case 3:           rt_kprintf(" BD+GPS");  
				  	              break;
		      	} 
     //   ¶¨Î»×´×´Ì¬
     if(ModuleStatus&Status_GPS)
	 	        rt_kprintf("         ¶¨Î»×´Ì¬:  ¶¨Î»   ÎÀÐÇ¿ÅÊý: %d ¿Å ",Satelite_num);   
    else	 
                      rt_kprintf("         ¶¨Î»×´Ì¬: Î´¶¨Î»");  	 
    //    GPS ÌìÏß×´Ì¬
    if(GpsStatus.Antenna_Flag==1)
         rt_kprintf("        ÌìÏß:     ¶Ï¿ª");  
   else	
   	  rt_kprintf("        ÌìÏß:     Õý³£");   
     //   GPRS  ×´Ì¬ 
    if(ModuleStatus&Status_GPRS)
	 	        rt_kprintf("      GPRS ×´Ì¬:   Online\r\n");  
    else	 
                      rt_kprintf("      GPRS ×´Ì¬:   Offline\r\n");  	   
 
}
FINSH_FUNCTION_EXPORT(sys_status, Status);
void dispdata(char* instr)
{
     if (strlen(instr)==0)
	{
	     DispContent=1;
	    rt_kprintf("\r\n  Ä¬ÈÏµÈÓÚ 1\r\n"); 	  
	    return ;
	}
	else 
	{         
	       DispContent=(instr[0]-0x30);    
	  rt_kprintf("\r\n		Dispdata =%d \r\n",DispContent); 
	  return;  
	}
}
FINSH_FUNCTION_EXPORT(dispdata, Debug disp set) ;

void Socket_main_Set(u8* str) 
{
  u8 i=0;
  u8 reg_str[80];
  
	if (strlen((const char*)str)==0){
	    rt_kprintf("\r\n input error\r\n"); 
		return ;
	}
	else 
	{      
	  i = str2ipport((char*)str, RemoteIP_main, &RemotePort_main);
	  if (i <= 4) return ;;
	   
	  memset(reg_str,0,sizeof(reg_str));
	  IP_Str((char*)reg_str, *( u32 * ) RemoteIP_main);		   
	  strcat((char*)reg_str, " :"); 	  
	  sprintf((char*)reg_str+strlen((const char*)reg_str), "%u\r\n", RemotePort_main);  
         memcpy((char*)SysConf_struct.IP_Main,RemoteIP_main,4);
	  SysConf_struct.Port_main=RemotePort_main;
	 Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));

        DataLink_MainSocket_set(RemoteIP_main,RemotePort_main,1);
		 DataLink_EndFlag=1; //AT_End();  
			return ;
	}

}
FINSH_FUNCTION_EXPORT(Socket_main_Set,Set Socket main); 


//==========================================================
 
/*    
     -----------------------------
    3.  RT Çý¶¯Ïà¹Ø
     ----------------------------- 
*/



/*

       ÐÂÇý¶¯Ó¦ÓÃ
 
*/



  //  1 .  Ñ­»·´æ´¢ 
      u8     Api_cycle_write(u8 *buffer, u16 len) 
      {
             WatchDog_Feed(); 
             if( SaveCycleGPS(cycle_write,buffer,len))
	     { //---- updata pointer   -------------		
			cycle_write++;  	
		       if(cycle_write>=Max_CycleNum)
		  	               cycle_write=0;  
			DF_Write_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd);      
	      //-------------------------------	
	        return true; 
            }  
	    else
		 return  false;	  
  	}

      u8      Api_cycle_read(u8 *buffer, u16 len) 
      {
                return( ReadCycleGPS(cycle_read, buffer, len));
  	}
       u8     Api_cycle_Update(void)
       {
           	 if( ReadCycle_status==RdCycle_SdOver)	 // ·¢ËÍ³É¹¦ÅÐ¶Ï 
				 {		 
					 cycle_read++;	 //  ÊÕµ½Ó¦´ð²ÅµÝÔö
					 if(cycle_read>=Max_CycleNum)
							cycle_read=0;
					 ReadCycle_status=RdCycle_Idle; 
					  rt_kprintf("\r\n ·¢ËÍ GPS --saved  OK --update!\r\n");    
					   ReadCycle_timer=0; 
				 }	
				 return 1;
      }
	u8   Api_CHK_ReadCycle_status(void)
	{
                  CHK_ReadCycle_status();    // ·¢ËÍ×´Ì¬µÄ¼ì²é
		     return true; 
	}

 // 2. Config 
        u8    Api_Config_write(u8 *name,u16 ID,u8* buffer, u16 wr_len)  
 	{
                if(strcmp((const char*)name,config)==0)
                {
                     DF_WriteFlashSector(ConfigStart_offset, 0, buffer, wr_len);
			DF_delay_ms(5);
			return true;		 
                }  
		   if(strcmp((const char*)name,tired_config)==0)
                {
                     DF_WriteFlashSector(TiredCondifg_offset, 0, buffer, wr_len);
			return true;		 
                } 		
		 return false;		
 	}
       u8      Api_Config_read(u8 *name,u16 ID,u8* buffer, u16 Rd_len)    //  ¶ÁÈ¡Media area ID ÊÇ±¨Êý
       {
               if(strcmp((const char*)name,config)==0)
           	{
                     DF_ReadFlash(ConfigStart_offset, 0, buffer, Rd_len); 
			DF_delay_ms(10); 	// large content delay	
			return true;		
           	}
              if(strcmp((const char*)name,jt808)==0) 
             	{
                     DF_ReadFlash(JT808Start_offset, 0, buffer, Rd_len); 
			DF_delay_ms(10); // large content delay
			return true;	
             	}
		   if(strcmp((const char*)name,tired_config)==0)
                {
                     DF_ReadFlash(TiredCondifg_offset, 0, buffer, Rd_len);
			DF_delay_ms(10); 		 
			return true;		 
                } 	  
     		   if(strcmp((const char*)name,BD_ext_config)==0)
     		   {
                     DF_ReadFlash(DF_BD_Extend_Page, 0, buffer, Rd_len); 
			DF_delay_ms(10); 		 
			return true;	
     		   }
         return false;
			
       }
 
         u8    Api_Config_Recwrite(u8 *name,u16 ID,u8* buffer, u16 wr_len)  // ¸üÐÂ×îÐÂ¼ÇÂ¼  
 	{
         return 1;
 	}

         u8    Api_Config_Recwrite_Large(u8 *name,u16 ID,u8* buffer, u16 wr_len)  // ¸üÐÂ×îÐÂ¼ÇÂ¼  
 	{
           if(strcmp((const char*)name,jt808)==0)
                {
                     DF_WriteFlashSector(JT808Start_offset, 0, buffer, wr_len);
			return true;		 
                }
	    if(strcmp((const char*)name,BD_ext_config)==0)
	    	{
                    DF_WriteFlashSector(DF_BD_Extend_Page,0, buffer, wr_len);
                    return true;
	    	}				
		 return false;	
 	}
	  
 //  3.  ÆäËû 
       u8    Api_DFdirectory_Create(u8* name, u16 sectorNum)  // 
 	{ 
              return true ;  // NO  action here
 	}
       void   Api_MediaIndex_Init(void)
       {
                // Api_DFdirectory_Create(pic_index,pic_index_size);    //  Í¼Æ¬Ë÷Òý
	          // Api_DFdirectory_Create(voice_index,voice_index_size);    //  ÉùÒôË÷?
                 MediaIndex_Init();
       }

       u32  Api_DFdirectory_Query(u8 *name, u8  returnType)
       {         //  returnType=0  ·µ»Ø¼ÇÂ¼ÊýÄ¿     returnType=1 Ê±·µ»Ø¸ÄÄ¿Â¼ÎÄ¼þ´óÐ¡
             u8   flag=0;
	      u32  pic_current_page=0;		 
			 
		 if(strcmp((const char*)name,spdpermin)==0)
		 	{return AvrgSpdPerMin_write;}
		  if(strcmp((const char*)name,tired_warn)==0)
		     {return TiredDrv_write ;}
		  if(strcmp((const char*)name,camera_1)==0)
		  {      pic_current_page=PicStart_offset;
		          flag=1;
		   }
		   if(strcmp((const char*)name,camera_2)==0)
		     {      pic_current_page=PicStart_offset2;
		          flag=1;
		   }
		   if(strcmp((const char*)name,camera_3)==0)
		   {      pic_current_page=PicStart_offset3;
		          flag=1;
		   }
		   if(strcmp((const char*)name,camera_4)==0)
			  {      pic_current_page=PicStart_offset4;
		          flag=1;
		   }
		   if(strcmp((const char*)name,voice)==0)
		   {      DF_ReadFlash(SoundStart_offdet,0,(u8*)&SrcFileSize,4);  
		           return SrcFileSize;
		   }
		   if(flag)
		   	{
		        DF_ReadFlash(pic_current_page, 0,PictureName, 23);
                      memcpy((u8*)&PicFileSize,PictureName+19,4);   
			 return   PicFileSize	;
		   }
		    return  0;
       }
	   
         u8 Api_DFdirectory_Write(u8 *name,u8 *buffer, u16 len) 
         {
		 
               if(strcmp((const char*)name,spdpermin)==0)
		  {
		            Save_PerMinContent(AvrgSpdPerMin_write,buffer, len);
			     //-----  Record update----		
			  AvrgSpdPerMin_write++;
			  if(AvrgSpdPerMin_write>=Max_SPDSperMin)
			     AvrgSpdPerMin_write=0;
			  DF_Write_RecordAdd(AvrgSpdPerMin_write, AvrgSpdPerMin_write, TYPE_AvrgSpdAdd);   
			     //----------------------	 
			return true;	 
               }
		 if(strcmp((const char*)name,spd_warn)==0) 
		{
			   Common_WriteContent( ExpSpdRec_write, buffer, len, TYPE_ExpSpdAdd);    
			   //-----  Record update----	
			  ExpSpdRec_write++;
			  if(ExpSpdRec_write>=Max_SPDSperMin)
			      ExpSpdRec_write=0;
	                DF_Write_RecordAdd(ExpSpdRec_write, ExpSpdRec_write, TYPE_ExpSpdAdd); 	  
			  //-----  Record update----	
			   return true;
               }
                if(strcmp((const char*)name,tired_warn)==0) 
		 {
			   Common_WriteContent( TiredDrv_write, buffer, len,  TYPE_TiredDrvAdd);      
			   //-----  Record update----	
			  TiredDrv_write++;
			  if(TiredDrv_write>=Max_CommonNum)  
			  	TiredDrv_write=0;			  
			  DF_delay_us(10);
			  DF_Write_RecordAdd(TiredDrv_write,TiredDrv_read,TYPE_TiredDrvAdd);   
			  //-------------------------
			  return  true;
                }
				
		 if(strcmp((const char*)name,doubt_data)==0)
		{ 
		       Save_DrvRecoder(Recorder_write, buffer, len );   
			//-----  Record update----	    
			   Recorder_write++; 
			   if(Recorder_write>=Max_RecoderNum)  
			   	 Recorder_write=0;
			   DF_Write_RecordAdd(Recorder_write,Recorder_Read,TYPE_VechRecordAdd); 	
		       //-------------------------		   
			return true;
               }	
		 //------- MultiMedia   RAW  data  ---------
		 if(strcmp((const char*)name,voice)==0)
		{
	            DF_WriteFlashDirect(SoundStart_offdet+Dev_Voice.Voice_PageCounter,0,Dev_Voice.Voice_Reg,500);  
                   return true;
		 }
		if(strcmp((const char*)name,camera_1)==0)
		{
                     DF_WriteFlashDirect(pic_current_page,0,buffer, len);
			return true;		 
               }
		if(strcmp((const char*)name,camera_2)==0)
		{
                     DF_WriteFlashDirect(pic_current_page,0,buffer, len);
			return true;		 
               }
		if(strcmp((const char*)name,camera_3)==0)
		{
                      DF_WriteFlashDirect(pic_current_page,0,buffer, len);
			return true;		  
               }
	       if(strcmp((const char*)name,camera_4)==0)
		{
                      DF_WriteFlashDirect(pic_current_page,0,buffer, len); 
			return true;		  
               }	 
		
              return false;
         }
          u8  Api_DFdirectory_Read(u8 *name,u8 *buffer, u16 len, u8  style ,u16 numPacket)  // style  1. old-->new   0 : new-->old 
         {   /*  Á¬Ðøµ÷ÓÃ¼¸´Î £¬ÒÀ´Î°´style ·½Ê½µÝÔö*/
               // style  1. old-->new   0 : new-->old 
               //   numPacket    : °²×° style  ·½Ê½¶ÁÈ¡¿ªÊ¼ µÚ¼¸ÌõÊý¾Ý°ü  from: 0
               u16   read_addr=0;
			   
              if(strcmp((const char*)name,spdpermin)==0)
		  {
		        if(style==1)
					read_addr=0+numPacket;
			 else
			 { 
			      if(AvrgSpdPerMin_write==0)
				  	    return   false; 
				else  
			      if(AvrgSpdPerMin_write>=(numPacket+1))		
			            read_addr=AvrgSpdPerMin_write-1-numPacket;
				else
					 return false;
			 }  
		            Read_PerMinContent(read_addr,buffer, len);
			     //----------------------	 
			return true;	 
               }
		 if(strcmp((const char*)name,spd_warn)==0) 
		{
			 if(style==1)
					read_addr=0+numPacket;
			 else
			 { 
			      if(ExpSpdRec_write==0)
				  	    return   false; 
				else  
			      if(ExpSpdRec_write>=(numPacket+1))		
			            read_addr=ExpSpdRec_write-1-numPacket;
				else
					 return false;
			 }  	
			   Common_ReadContent( read_addr, buffer, len, TYPE_ExpSpdAdd);    
			   return true;
               }
                if(strcmp((const char*)name,tired_warn)==0) 
		 {
			   if(style==1)
					read_addr=0+numPacket;
			 else
			 { 
			      if(TiredDrv_write==0)
				  	    return   false; 
				else  
			      if(TiredDrv_write>=(numPacket+1))		
			            read_addr=TiredDrv_write-1-numPacket;
				else
					 return false;
			 } 
			   Common_ReadContent( read_addr, buffer, len,  TYPE_TiredDrvAdd);      

			  //-------------------------
			  return  true;
                }
				
		 if(strcmp((const char*)name,doubt_data)==0)
		{ 
		        if(style==1)
					read_addr=0+numPacket;
			 else
			 { 
			      if(Recorder_write==0)
				  	    return   false; 
				else  
			      if(Recorder_write>=(numPacket+1))		
			            read_addr=Recorder_write-1-numPacket;
				else
					 return false;
			 }  
		       Read_DrvRecoder(read_addr, buffer, len );   
	
		       //-------------------------		   
			return true;
               }	
		 //------- MultiMedia   RAW  data  ---------
		 if(strcmp((const char*)name,voice)==0)
		{
		     if(style==0)
					return  false;  //  Ö»ÔÊÐí´Óold  -> new
	            DF_ReadFlash(SoundStart_offdet+numPacket,0, buffer, len);  
                   return true;
		 }
		if(strcmp((const char*)name,camera_1)==0)
		{
                     DF_ReadFlash(PicStart_offset+numPacket,0,buffer, len);
			return true;		 
               }
		if(strcmp((const char*)name,camera_2)==0)
		{
                     DF_ReadFlash(PicStart_offset2+numPacket,0,buffer, len);
			return true;		 
               }
		if(strcmp((const char*)name,camera_3)==0)
		{
                      DF_ReadFlash(PicStart_offset3+numPacket,0,buffer, len);
			return true;		  
               }
	       if(strcmp((const char*)name,camera_4)==0)
		{
                      DF_ReadFlash(PicStart_offset4+numPacket,0,buffer, len); 
			return true;		  
               }	 
		
              return false;
         }

	u8    Api_DFdirectory_Delete(u8* name)
 	{ 
	   
              if(strcmp((const char*)name,voice)==0)
		{
		                 WatchDog_Feed();    
				   SST25V_BlockErase_32KByte((SoundStart_offdet<<9));
				   DF_delay_ms(300);  
				   WatchDog_Feed(); 
			    return true;
               }
		if(strcmp((const char*)name,camera_1)==0)
		{
		       WatchDog_Feed(); 
                     SST25V_BlockErase_64KByte((PicStart_offset<<9));      
			DF_delay_ms(500);   	
			WatchDog_Feed(); 
			return true;		 
               }
		if(strcmp((const char*)name,camera_2)==0)
		{
		       WatchDog_Feed(); 
                     SST25V_BlockErase_64KByte((PicStart_offset2<<9));   
			DF_delay_ms(500);   	
			WatchDog_Feed(); 
			return true;			 
               }
		if(strcmp((const char*)name,camera_3)==0)
		{
		         WatchDog_Feed(); 
                       SST25V_BlockErase_64KByte((PicStart_offset3<<9));    
			  DF_delay_ms(500);   
			  WatchDog_Feed(); 
			  return true;	 		   
               }
	       if(strcmp((const char*)name,camera_4)==0)
		{
		        WatchDog_Feed(); 
                       SST25V_BlockErase_64KByte((PicStart_offset4<<9));   
		         DF_delay_ms(500);   
			  WatchDog_Feed(); 	 
			  return true;			   
               }  
		  return false; 
 	}
	  
         u8   Api_DFdirectory_DelteAll(void)  
         {
              return 1;  // no action  here
         }  
//-------  ¹Ì¶¨Î»ÖÃ ÐòºÅ¼ÇÂ¼  -----------
  u8   Api_RecordNum_Write( u8 *name,u8 Rec_Num,u8 *buffer, u16 len)    //  Rec_Num<128  Len<128
{
          WatchDog_Feed();
           if(strcmp((const char*)name,event_808)==0)
           	{
                   DF_WriteFlash(DF_Event_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
           if(strcmp((const char*)name,msg_broadcast)==0)
		{
                   DF_WriteFlash(DF_Broadcast_offset+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}   	
	    if(strcmp((const char*)name,phonebook)==0)
		{
                   DF_WriteFlash(DF_PhoneBook_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	
	    if(strcmp((const char*)name,Rail_cycle)==0)
		{
                   DF_WriteFlash(DF_Event_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	
           if(strcmp((const char*)name,Rail_rect)==0)
	        {
                   DF_WriteFlash(DF_RectangleRail_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		   	
	    if(strcmp((const char*)name,Rail_polygen)==0)	
	     {
                   DF_WriteFlash(DF_PolygenRail_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}			
	    if(strcmp((const char*)name,turn_point)==0)
		{
                   DF_WriteFlash(DF_turnPoint_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		
           if(strcmp((const char*)name,route_line)==0)
		{
                   DF_WriteFlash(DF_Route_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	   	
	    if(strcmp((const char*)name,ask_quesstion)==0)	
		{
                   DF_WriteFlash(DF_AskQuestion_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		
	    if(strcmp((const char*)name,text_msg)==0)
	      {
                   DF_WriteFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
  //-------------------------------
                if(strcmp((const char*)name,pic_index)==0)
                {
                            DF_WriteFlash(DF_PicIndex_Page+Rec_Num, 0,buffer, len); 
				return true;			
                }
		 if(strcmp((const char*)name,voice_index)==0)
		{
                            DF_WriteFlash(DF_SoundIndex_Page+Rec_Num, 0,buffer, len);
				return  true;			
              }
		 
		 return false;
}

  u8   Api_RecordNum_Read( u8 *name,u8 Rec_Num,u8 *buffer, u16 len)    //  Rec_Num<128  Len<128
  	{

             if(strcmp((const char*)name,event_808)==0)
           	{
                   DF_ReadFlash(DF_Event_Page+Rec_Num, 0,buffer, len);    
		    // DF_delay_ms(10);   		   
		     return true;		   
           	}
           if(strcmp((const char*)name,msg_broadcast)==0)
		{
                   DF_ReadFlash(DF_Broadcast_offset+Rec_Num, 0,buffer, len);    
                   //DF_delay_ms(10);   				   
		     return true;		   
           	}   	
	    if(strcmp((const char*)name,phonebook)==0)
		{
                 DF_ReadFlash(DF_PhoneBook_Page+Rec_Num, 0,buffer, len);   
		   //DF_delay_ms(10);   		   
		     return true;		   
           	}	
	    if(strcmp((const char*)name,Rail_cycle)==0)
		{
                   DF_ReadFlash(DF_Event_Page+Rec_Num, 0,buffer, len);   
		    // DF_delay_ms(10);   		   
		     return true;		   
           	}	
           if(strcmp((const char*)name,Rail_rect)==0)
	        {
                   DF_ReadFlash(DF_RectangleRail_Page+Rec_Num, 0,buffer, len);   
		    // DF_delay_ms(10);   		   
		     return true;		   
           	}		   	
	    if(strcmp((const char*)name,Rail_polygen)==0)	
	     {
                   DF_ReadFlash(DF_PolygenRail_Page+Rec_Num, 0,buffer, len);   
		     //DF_delay_ms(10);   		   
		     return true;		   
           	}			
	    if(strcmp((const char*)name,turn_point)==0)
		{
                   DF_ReadFlash(DF_turnPoint_Page+Rec_Num, 0,buffer, len);   
		    // DF_delay_ms(10);   		   
		     return true;		   
           	}		
           if(strcmp((const char*)name,route_line)==0)
		{
                   DF_ReadFlash(DF_Route_Page+Rec_Num, 0,buffer, len);   
		    DF_delay_ms(10); 		   
		     return true;		   
           	}	   	
	    if(strcmp((const char*)name,ask_quesstion)==0)	
		{
                   DF_ReadFlash(DF_AskQuestion_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		
	    if(strcmp((const char*)name,text_msg)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     //DF_delay_ms(10); 		   
		     return true;		   
           	}
  	      if(strcmp((const char*)name,pic_index)==0)
                {
                            DF_ReadFlash(DF_PicIndex_Page+Rec_Num, 0,buffer, len);  
				return true;			
                }
		 if(strcmp((const char*)name,voice_index)==0)
		{
                            DF_ReadFlash(DF_SoundIndex_Page+Rec_Num, 0,buffer, len);
				//DF_delay_ms(10); 			
				return  true;			
              }
		 return false;

  	}
    u16  Api_RecordNum_Query(u8 *name) 
       {
       #if 0
             if(strcmp((const char*)name,event_808)==0)
           	{
                       
		     return event;		   
           	}
           if(strcmp((const char*)name,msg_broadcast)==0)
		{
                   DF_ReadFlash(DF_Broadcast_offset+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}   	
	    if(strcmp((const char*)name,phonebook)==0)
		{
                   DF_ReadFlash(DF_PhoneBook_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	
	    if(strcmp((const char*)name,Rail_cycle)==0)
		{
                   DF_ReadFlash(DF_Event_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	
           if(strcmp((const char*)name,Rail_rect)==0)
	        {
                   DF_ReadFlash(DF_RectangleRail_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		   	
	    if(strcmp((const char*)name,Rail_polygen)==0)	
	     {
                   DF_ReadFlash(DF_PolygenRail_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}			
	    if(strcmp((const char*)name,turn_point)==0)
		{
                   DF_ReadFlash(DF_turnPoint_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		
           if(strcmp((const char*)name,route_line)==0)
		{
                   DF_ReadFlash(DF_Route_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}	   	
	    if(strcmp((const char*)name,ask_quesstion)==0)	
		{
                   DF_ReadFlash(DF_AskQuestion_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}		
	    if(strcmp((const char*)name,text_msg)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}



 // 3.  ¼ÇÂ¼ 
                if(strcmp((const char*)name,spd_warn)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
		    if(strcmp((const char*)name,tired_warn)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
		    if(strcmp((const char*)name,doubt_data)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
		    if(strcmp((const char*)name,spdpermin)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
		    if(strcmp((const char*)name,pospermin)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
		    if(strcmp((const char*)name,pic_index)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
		    if(strcmp((const char*)name,voice_index)==0)
	      {
                   DF_ReadFlash(DF_Msg_Page+Rec_Num, 0,buffer, len);   
		     return true;		   
           	}
              return  false;
	  #endif		  
						return 1;
       }

  void  debug_relay(u8 *str) 
{
 if (strlen(str)==0)
	{
       rt_kprintf("\r\n¼ÌµçÆ÷(1:¶Ï¿ª0:±ÕºÏ)JT808Conf_struct.relay_flag=%d",JT808Conf_struct.relay_flag);
       }
else 
	{
	       if(str[0]=='1')
		{
		 Car_Status[2]|=0x08;     // ÐèÒª¿ØÖÆ¼ÌµçÆ÷
		JT808Conf_struct.relay_flag=1;
		Enable_Relay();
		rt_kprintf("\r\n  ¶Ï¿ª¼ÌµçÆ÷,JT808Conf_struct.relay_flag=%d\r\n",JT808Conf_struct.relay_flag); 
		}
	else if(str[0]=='0')
		{
		Car_Status[2]&=~0x08;    // ÐèÒª¿ØÖÆ¼ÌµçÆ÷
		JT808Conf_struct.relay_flag=0;
		Disable_Relay();
		rt_kprintf("\r\n  ½ÓÍ¨¼ÌµçÆ÷,JT808Conf_struct.relay_flag=%d\r\n",JT808Conf_struct.relay_flag); 
		}
	}
 Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct)); 
 rt_kprintf("\r\n(debug_relay)×´Ì¬ÐÅÏ¢,[0]=%X  [1]=%X  [2]=%X  [3]=%X",Car_Status[0],Car_Status[1],Car_Status[2],Car_Status[3]);	
 }
FINSH_FUNCTION_EXPORT(debug_relay, Debug relay set) ;
	
    void  Api_WriteInit_var_rd_wr(void)    //   Ð´³õÊ¼»¯»°¸÷ÀàÐÍ¶ÁÐ´¼ÇÂ¼µØÖ·
    	{
		 DF_Write_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd); 
		 DF_delay_ms(50);   
		 DF_Write_RecordAdd(pic_write,pic_read,TYPE_PhotoAdd);
		 DF_delay_ms(50);
		// DF_Write_RecordAdd(AvrgSpdPerSec_write,AvrgSpdPerSec_Read,TYPE_AvrgSpdSecAdd);
		// DF_delay_ms(50);  
		 //DF_Write_RecordAdd(Login_write,Login_Read,TYPE_LogInAdd);  
		// DF_delay_ms(50);  
		// DF_Write_RecordAdd(Powercut_write,Powercut_read,TYPE_PowerCutAdd);
		// DF_delay_ms(50);  
		 DF_Write_RecordAdd(Settingchg_write,Settingchg_read,TYPE_SettingChgAdd);
		 DF_delay_ms(50);  						 
		 DF_Write_RecordAdd(AvrgMintPosit_write,AvrgMintPosit_Read,TYPE_MintPosAdd); 
		 DF_delay_ms(50);  
		 
               DF_Write_RecordAdd(Distance_m_u32,DayStartDistance_32,TYPE_DayDistancAdd); 
	 	 DF_Write_RecordAdd(ExpSpdRec_write,ExpSpdRec_read,TYPE_ExpSpdAdd);  
	 	 DF_delay_ms(50); 
	 	 //DF_Write_RecordAdd(OnFireRec_write,OnFireRec_read,TYPE_AccFireAdd); 
	 	 //DF_delay_ms(50); 
		 
	 	 DF_Write_RecordAdd(TiredDrv_write,TiredDrv_read,TYPE_TiredDrvAdd); 
	 	 DF_delay_ms(50); 
	 	 DF_Write_RecordAdd(AvrgSpdPerMin_write,AvrgSpdPerMin_Read,TYPE_AvrgSpdAdd); 
		
		Recorder_write=0;
		Recorder_Read=0;  
		DF_Write_RecordAdd(Recorder_write,Recorder_Read,TYPE_VechRecordAdd); 



    	}
      void  Api_Read_var_rd_wr(void)    //   ¶Á³õÊ¼»¯»°¸÷ÀàÐÍ¶ÁÐ´¼ÇÂ¼µØÖ·
    	{
             DF_Read_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd); 
	      DF_delay_ms(50); 
		DF_Read_RecordAdd(pic_write,pic_read,TYPE_PhotoAdd);    
		 DF_delay_ms(50); 
		//DF_Read_RecordAdd(AvrgSpdPerSec_write,AvrgSpdPerSec_Read,TYPE_AvrgSpdSecAdd);
		//DF_Read_RecordAdd(Login_write,Login_Read,TYPE_LogInAdd);  
		//DF_Read_RecordAdd(Powercut_write,Powercut_read,TYPE_PowerCutAdd);
		DF_Read_RecordAdd(Settingchg_write,Settingchg_read,TYPE_SettingChgAdd);
		DF_Read_RecordAdd(AvrgMintPosit_write,AvrgMintPosit_Read,TYPE_MintPosAdd); 

               DF_Read_RecordAdd(Distance_m_u32,DayStartDistance_32,TYPE_DayDistancAdd); 
	 	 DF_Read_RecordAdd(ExpSpdRec_write,ExpSpdRec_read,TYPE_ExpSpdAdd);  
	 	 DF_delay_ms(50); 
	 	// DF_Read_RecordAdd(OnFireRec_write,OnFireRec_read,TYPE_AccFireAdd); 
	 	
		 
	 	 DF_Read_RecordAdd(TiredDrv_write,TiredDrv_read,TYPE_TiredDrvAdd); 
	 	 DF_delay_ms(50); 
	 	 DF_Read_RecordAdd(AvrgSpdPerMin_write,AvrgSpdPerMin_Read,TYPE_AvrgSpdAdd); 
 		 DF_Read_RecordAdd(Recorder_write,Recorder_Read,TYPE_VechRecordAdd); 


    	}

//---------------------------------------------------
  u8    ISP_Read( u32  Addr, u8*  Instr, u16 len)
{
       DF_ReadFlash(Addr,0,Instr,len);   
       return 1;	
}


u8     ISP_Write( u32  Addr, u8*  Instr, u16 len)
{         
      DF_WriteFlash(Addr,0,Instr,len);
	    return 1;
}

u8   ISP_Format(u16 page_counter,u16 page_offset,u8 *p,u16 length)
{
      u16 i=0;
    if(51==page_counter) 
    {
          DF_LOCK=enable;
       /*
           1.ÏÈ²Á³ý0ÉÈÇø   
           2.¶Á³öpage48ÄÚÈÝ²¢½«ÆäÐ´µ½µÚ0ÉÈÇøµÄpage 0 
           3.¶Á³öpage49ÄÚÈÝ²¢½«ÆäÐ´µ½µÚ0ÉÈÇøµÄpage 0
           4.²Á³ý 6-38ÉÈÇø ¼´ 48page µ½ 304 page
           5.ÒÔºóÓÐÊý¾Ý¹ýÀ´¾ÍÖ±½ÓÐ´Èë£¬²»ÐèÒªÔÙ²Á³ýÁË 
        */
	  DF_EraseAppFile_Area();  
    
	for(i=0;i<length;i++)
	{
		SST25V_ByteWrite(*p,(u32)page_counter*PageSIZE+(u32)(page_offset+i));
		p++;
	}
	//DF_delay_ms(5);
	  DF_LOCK=disable;
     }	
		 return 1;
}



//----------   TF ¿¨¼ì²é×´Ì¬
u8     TF_Card_Status(void)
{                    //    1:  succed             0:  fail
           return 0 ;

}


		 
