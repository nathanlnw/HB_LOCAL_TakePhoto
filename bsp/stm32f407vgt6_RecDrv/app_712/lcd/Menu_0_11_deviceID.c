#include  <string.h>
#include "Menu_Include.h"

#define  Sim_width1  6


static u8 deviceID_SetFlag=1;
static u8 deviceID_SetCounter=0;
static u8 deviceID_Screen=0;

unsigned char select_deviceID[]={0x0C,0x06,0xFF,0x06,0x0C};
unsigned char ABC_deviceID[10][1]={"0","1","2","3","4","5","6","7","8","9"};


DECL_BMP(8,5,select_deviceID);


void deviceID_Set(u8 par)
{
	lcd_fill(0);
	lcd_text12(0,3,(char *)Menu_deviceID_Code,deviceID_SetFlag-1,LCD_MODE_SET);
	lcd_bitmap(par*Sim_width1, 14, &BMP_select_deviceID, LCD_MODE_SET);
	lcd_text12(0,19,"0123456789",10,LCD_MODE_SET);
	lcd_update_all();
}

static void msg( void *p)
{

}
static void show(void)
{
CounterBack=0;
deviceID_SetCounter=0;
if(deviceID_Screen==0)
	deviceID_Screen=1;
lcd_fill(0);
lcd_text12(30,3,"设置入网ID",10,LCD_MODE_SET);
lcd_text12(0,19,"请确认是否需要重设置",20,LCD_MODE_SET);
lcd_update_all();
}


static void keypress(unsigned int key)
{
u8 i=0;
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_1_Idle;
			pMenuItem->show();
			memset(Menu_deviceID_Code,0,sizeof(Menu_deviceID_Code));
			deviceID_SetFlag=1;
			deviceID_SetCounter=0;
			deviceID_Screen=0;
			break;
		case KeyValueOk:;
			if(deviceID_Screen==1)
				{
				deviceID_Screen=2;
				lcd_fill(0);
				lcd_text12(0,3,"按确认键开始重新设置",20,LCD_MODE_SET);
				lcd_text12(24,19,"按菜单键退出",12,LCD_MODE_SET);
				lcd_update_all();
				}
			else if(deviceID_Screen==2)
				{
				deviceID_Screen=3;
				deviceID_Set(deviceID_SetCounter);
				}
			else if(deviceID_Screen==3)
				{
				if((deviceID_SetFlag>=1)&&(deviceID_SetFlag<=12))
					{
					Menu_deviceID_Code[deviceID_SetFlag-1]=ABC_deviceID[deviceID_SetCounter][0];
					deviceID_SetFlag++;	
					deviceID_SetCounter=0;
					deviceID_Set(0);
					}		
				else if(deviceID_SetFlag==13)
					{
					rt_kprintf("\r\n------------------- deviceID=%d,%s(ok)",deviceID_SetFlag,Menu_deviceID_Code);
					deviceID_SetFlag=14;
					
					memset(DeviceNumberID,0,sizeof(DeviceNumberID));
					memcpy(DeviceNumberID,Menu_deviceID_Code,12);								 
					DF_WriteFlashSector(DF_DeviceID_offset,0,DeviceNumberID,13); 
					delay_ms(80); 		  
					DF_ReadFlash(DF_DeviceID_offset,0,DeviceNumberID,13);    
					DeviceID_Convert_SIMCODE();  // 转换 
					rt_kprintf("\r\n--------------------密码进入设置入网ID为 : ");  
					for(i=0;i<12;i++)
					rt_kprintf("%c",DeviceNumberID[i]);
					rt_kprintf("\r\n");
					
					lcd_fill(0);
					lcd_text12(24,5,(char *)Menu_deviceID_Code,12,LCD_MODE_SET);
					lcd_text12(18,19,"入网ID设置完成",14,LCD_MODE_SET);
					lcd_update_all();	
					}
				else if(deviceID_SetFlag==14)
					{
					//rt_kprintf("\r\n deviceID_SetFlag=%d,%s(return)",deviceID_SetFlag,Menu_deviceID_Code);
					deviceID_SetFlag=1;
					deviceID_SetCounter=0;
					deviceID_Screen=0;

					pMenuItem=&Menu_1_Idle;
					pMenuItem->show();
					}
				}			
			break;
		case KeyValueUP:
			if(deviceID_Screen!=0)
				{
				if((deviceID_SetFlag>=1)&&(deviceID_SetFlag<=12))
					{
					if(deviceID_SetCounter==0)
						deviceID_SetCounter=9;
					else if(deviceID_SetCounter>=1)
						deviceID_SetCounter--;
					deviceID_Set(deviceID_SetCounter);
					}
				}
			break;
		case KeyValueDown:
			if(deviceID_Screen!=0)
				{
				if((deviceID_SetFlag>=1)&&(deviceID_SetFlag<=12))
					{
					if(deviceID_SetCounter==9)
						deviceID_SetCounter=0;
					else
						deviceID_SetCounter++;
					deviceID_Set(deviceID_SetCounter);
					}
				}
			break;
		}
	KeyValue=0;
}


static void timetick(unsigned int systick)
{
	/*CounterBack++;
	if(CounterBack!=MaxBankIdleTime*5)
		return;
	CounterBack=0;
	pMenuItem=&Menu_0_loggingin;
	pMenuItem->show();

	deviceID_SetFlag=1;
	deviceID_SetCounter=0;*/
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_0_11_deviceID=
{
"入网ID设置",
	10,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


