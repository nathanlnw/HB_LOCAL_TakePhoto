#include "Menu_Include.h"
#include "App_moduleConfig.h"


u8 RertartGps_screen=0;
u8 BDGPS_mode=0;//    1:冷启动    2:模式切换
u8 BDGPS_counter=0;//   

void mode_sel(void)
{
lcd_fill(0);
switch(BDGPS_counter)
	{
	case 0:
		lcd_text12(12,10,"双模",4,LCD_MODE_INVERT);
		lcd_text12(48,10,"北斗  GPS",9,LCD_MODE_SET);
		break;
	case 1:
		lcd_text12(12,10,"双模  ",6,LCD_MODE_SET);
		lcd_text12(48,10,"北斗",4,LCD_MODE_INVERT);
		lcd_text12(84,10,"GPS",3,LCD_MODE_SET);
		break;
	case 2:
		lcd_text12(12,10,"双模  北斗  ",12,LCD_MODE_SET);
		lcd_text12(84,10,"GPS",3,LCD_MODE_INVERT);
		break;
	}
lcd_update_all();
}




static void msg( void *p)
{
}
static void show(void)
{
	lcd_fill(0);
	lcd_text12(12,3,"1.冷启动模块",12,LCD_MODE_INVERT);
	lcd_text12(12,18,"2.工作模式设置",14,LCD_MODE_SET);
	lcd_update_all();
	
	RertartGps_screen=0;
	BDGPS_mode=1;
}


static void keypress(unsigned int key)
{

	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_5_other;
			pMenuItem->show();
			CounterBack=0;

			RertartGps_screen=0;
			BDGPS_mode=0;//    1:冷启动    2:模式切换
			BDGPS_counter=0;// 
			break;
		case KeyValueOk:
			if(RertartGps_screen==0)
				{
				if(BDGPS_mode==1)
					{
					RertartGps_screen=1;
					BDGPS_mode=0;
					lcd_fill(0);
					lcd_text12(6,10,"北斗模块冷启动成功",18,LCD_MODE_INVERT);
					lcd_update_all();
					
					 //---- 全冷启动 ------
					 /*
	                                  $CCSIR,1,1*48
	                                  $CCSIR,2,1*48 
	                                  $CCSIR,3,1*4A 
					 */		    
					if(GpsStatus.Position_Moule_Status==1)
						{
						gps_mode("1");
						rt_kprintf("\r\n北斗模式下冷启动");
						}
					else if(GpsStatus.Position_Moule_Status==2)
						{
						gps_mode("2");
						rt_kprintf("\r\nGPS模式下冷启动");
						}
					else if(GpsStatus.Position_Moule_Status==3)
						{
						gps_mode("3");
						rt_kprintf("\r\n双模下冷启动");
						}
					}
				else if(BDGPS_mode==2)
					{
					RertartGps_screen=2;
					lcd_fill(0);
					lcd_text12(12,10,"双模",4,LCD_MODE_INVERT);
					lcd_text12(48,10,"北斗  GPS",9,LCD_MODE_SET);
					lcd_update_all();
					}
				}
			else if(RertartGps_screen==2)
				{
				RertartGps_screen=3;
				lcd_fill(0);
				if(BDGPS_counter==0)
					{
					GpsStatus.Position_Moule_Status=3;
					lcd_text12(20,10,"选择模式:双模",13,LCD_MODE_INVERT);
				       gps_mode("3");
					}
				else if(BDGPS_counter==1)
					{
					GpsStatus.Position_Moule_Status=1;
					lcd_text12(20,10,"选择模式:北斗",13,LCD_MODE_INVERT);
					gps_mode("1");
					}
				else if(BDGPS_counter==2)
					{
					GpsStatus.Position_Moule_Status=2;
					lcd_text12(20,10,"选择模式:GPS",12,LCD_MODE_INVERT);
					gps_mode("2");
					}
				lcd_update_all();
				}
			break;
		case KeyValueUP:
			if(RertartGps_screen==0)
				{
				BDGPS_mode=1;
				lcd_fill(0);
				lcd_text12(12,3,"1.冷启动模块",12,LCD_MODE_INVERT);
				lcd_text12(12,18,"2.工作模式设置",14,LCD_MODE_SET);
				lcd_update_all();
				}
			else if(RertartGps_screen==2)
				{
				if(BDGPS_counter==0)
					BDGPS_counter=2;
				else
					BDGPS_counter--;
				mode_sel();
				}
			break;
		case KeyValueDown:
			if(RertartGps_screen==0)
				{
				BDGPS_mode=2;
				lcd_fill(0);
				lcd_text12(12,3,"1.冷启动模块",12,LCD_MODE_SET);
				lcd_text12(12,18,"2.工作模式设置",14,LCD_MODE_INVERT);
				lcd_update_all();
				}
			else if(RertartGps_screen==2)
				{
				if(BDGPS_counter==2)
					BDGPS_counter=0;
				else
					BDGPS_counter++;
				mode_sel();
				}
			break;
		}
 KeyValue=0;
}


static void timetick(unsigned int systick)
{
	Cent_To_Disp(); 
	CounterBack++;
	if(CounterBack!=MaxBankIdleTime*5)
		return;
	CounterBack=0;
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();
	
	RertartGps_screen=0;
	BDGPS_mode=0;//    1:冷启动    2:模式切换
	BDGPS_counter=0;// 
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_5_4_bdColdBoot=
{
"北斗模块设置",
	12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};

