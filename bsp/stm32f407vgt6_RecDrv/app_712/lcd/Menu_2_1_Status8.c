#include "Menu_Include.h"
#include "Lcd.h"

u8 tickcount_sig=0;

static void msg( void *p)
{
char *pinfor;
char len=0;

pinfor=(char *)p;
len=strlen(pinfor);

lcd_fill(0);
lcd_text12(0,10,pinfor,len,LCD_MODE_SET);
lcd_update_all();
}
static void show(void)
{
	msg(XinhaoStatus);
}


static void keypress(unsigned int key)
{
	switch(KeyValue)
		{
		case KeyValueMenu:
			pMenuItem=&Menu_2_InforCheck;
			pMenuItem->show();
			CounterBack=0;
			break;
		case KeyValueOk:
			msg(XinhaoStatus);
			break;
		case KeyValueUP:
			msg(XinhaoStatus);
			break;
		case KeyValueDown:
			msg(XinhaoStatus);
			break;
		}
 KeyValue=0;
}


static void timetick(unsigned int systick)
{
    //循环显示
	tickcount_sig++;
	if(tickcount_sig>=6) 
		{
		tickcount_sig=0;
	    msg(XinhaoStatus);
		}
	
	CounterBack++;
	if(CounterBack!=MaxBankIdleTime)
		return;
	CounterBack=0;
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_2_1_Status8=
{
    "信号线状态",
	10,
	&show,
	&keypress,
	&timetick,
	&msg,
	(void*)0
};


