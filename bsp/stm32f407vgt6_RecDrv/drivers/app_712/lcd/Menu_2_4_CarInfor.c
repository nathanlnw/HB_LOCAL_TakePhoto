#include  <string.h>
#include "Menu_Include.h"
#include "Lcd.h"

static unsigned char vech_num[16]={"���ƺ�:        "};
static unsigned char vech_type[25]={"��������:            "};
static unsigned char  vech_ID[19]={"����ID:            "}; 
static unsigned char  vech_VIN[20]={"VIN                 "}; 
static unsigned char  vech_login[20]={"                    "}; //vech_login
static unsigned char updown_flag=0;

//��ʻԱ����
void Display_driver(u8 drivercar)
{
    u8 color_disp[4];
switch(drivercar)
	{
	case 1:
	
		memcpy(vech_num+7,JT808Conf_struct.Vechicle_Info.Vech_Num,8); 
		//rt_kprintf("\r\n1.vech_num=%s,JT808Conf_struct.Vechicle_Info.Vech_Num=%s,Menu_Car_license=%s",vech_num,JT808Conf_struct.Vechicle_Info.Vech_Num,Menu_Car_license);
		
            //������ɫ
               memset(color_disp,0,sizeof(color_disp));
               switch(JT808Conf_struct.Vechicle_Info.Dev_Color)
               	{
                case 1: memcpy(color_disp,"��ɫ",4);break;
                case 2:memcpy(color_disp,"��ɫ",4);break;
			    case 3:memcpy(color_disp,"��ɫ",4);break;
			    case 4:memcpy(color_disp,"��ɫ",4);break;
			    case 9:memcpy(color_disp,"����",4);break;
			    default:memcpy(color_disp,"��ɫ",4);break;	
               	}
        lcd_fill(0);
        lcd_text12(10,3,(char *)vech_num,15,LCD_MODE_SET);	   
        lcd_text12(10,19,"������ɫ:",9,LCD_MODE_SET);
        lcd_text12(64,19,(char *)color_disp,4,LCD_MODE_SET); 
        lcd_update_all();
    	//rt_kprintf("\r\n2.vech_num=%s,JT808Conf_struct.Vechicle_Info.Vech_Num=%s,Menu_Car_license=%s",vech_num,JT808Conf_struct.Vechicle_Info.Vech_Num,Menu_Car_license);
		break;

	case 2://����ID Vechicle_Info.DevicePhone
		lcd_fill(0);
              memcpy(vech_type+9,JT808Conf_struct.Vechicle_Info.Vech_Type,6);
		lcd_text12(0,3,(char *)vech_type,19,LCD_MODE_SET);		
		//��ȡ�豸�ٶ�ȡ����GPS�ٶȻ����ٶ����ٶ�
		/*DF_ReadFlash(DF_Speed_GetType_Page,0,(u8*)&Speed_GetType,1);
		if(Speed_GetType==0)*/
			lcd_text12(0,19,"�豸�ٶ�:GPS�ٶ�",16,LCD_MODE_SET);
		/*else if(Speed_GetType==1)
			lcd_text12(0,5,"�豸�ٶ�:�������ٶ�",19,LCD_MODE_SET);*/
		lcd_update_all();
		break;
	case 3:  //  ����ID
		lcd_fill(0);
		memcpy(vech_ID+7,SIM_CardID_JT808,12); //����ID   ,������ֻ�����
		lcd_text12(0,3,(char *)vech_ID,19,LCD_MODE_SET);
		memcpy(vech_VIN+3,JT808Conf_struct.Vechicle_Info.Vech_VIN,17); //����VIN
		lcd_text12(0,19,(char *)vech_VIN,20,LCD_MODE_SET);
		lcd_update_all();   
		 break;
	 case 4:  //  ����ID
 		memcpy(vech_login+7,JT808Conf_struct.ConfirmCode,strlen(JT808Conf_struct.ConfirmCode)); //����VIN
		lcd_fill(0);
		lcd_text12(0,3,"��Ȩ��",6,LCD_MODE_SET);
		lcd_text12(0,18,(char *)vech_login,strlen(JT808Conf_struct.ConfirmCode)+7,LCD_MODE_SET);
		lcd_update_all();   
		 break;
	default:
		break;
	}

}

static void msg( void *p)
{
}
static void show(void)
{
	lcd_fill(0);
	lcd_text12(24, 3,"������Ϣ�鿴",12,LCD_MODE_SET);
	lcd_text12(24,19,"�鿴�밴ѡ��",12,LCD_MODE_SET);
	lcd_update_all();

}


static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_2_InforCheck;
			pMenuItem->show();
			CounterBack=0;

			updown_flag=0;
			break;
		case KeyValueOk:
			updown_flag=1;
			Display_driver(1);
			break;
		case KeyValueUP:
                    if(updown_flag==1)
				updown_flag=4;
			else
				updown_flag--;
			Display_driver(updown_flag);
			break;
		case KeyValueDown:
                   if(updown_flag==4)
				updown_flag=1;
		      else
				updown_flag++;
			Display_driver(updown_flag);
			break;
		}
	KeyValue=0;
}


static void timetick(unsigned int systick)
{
    Cent_To_Disp();
	CounterBack++;
	if(CounterBack!=MaxBankIdleTime)
		return;
	CounterBack=0;
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();
}


ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_2_4_CarInfor=
{
	"������Ϣ�鿴",
	12,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


