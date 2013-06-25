/*
     APP_GSM.C 
*/

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "App_gsm.h"
#include "SMS.h"



/* gsm thread */

ALIGN(RT_ALIGN_SIZE)
char gsm_thread_stack[4096];    
struct rt_thread gsm_thread; 

struct rt_semaphore gsmRx_sem;


#ifdef RT_USING_DEVICE 
 struct rt_device  Device_GSM;
#endif

/* 定时器的控制块 */
 static rt_timer_t timer_gsm; 


//-----  GSM  消息邮箱------
 static MSG_Q_TYPE msgq_gsm;
 u8  GSM_HEX[1024];  
u16  GSM_HEX_len=0; 


//----- gsm_thread  rx   app_thread  data  related ----- 	
static  MSG_Q_TYPE  gsm_rx_app_infoStruct;  //  gsm  接收从 app 来的数据结构
static  struct rt_semaphore gsmSd_Appdata_sem;  //  app 提供数据 给gsm发送信号量

ALIGN(RT_ALIGN_SIZE)
 uint8_t					GSM_rawinfo[GSM_RAWINFO_SIZE];
 struct rt_messagequeue	mq_GSM; 




/*
    应用相关  
*/
 u8  DataLink_Online=0;   //  DataLink  在线标志    以前的GPRS_Online
 u8  DataLink_EndFlag=0;         // Close_DataLink
 u8  DataLink_EndCounter=0; 
 u8   PositionInfo_sdFlag=0;     // 发送定位信息标志
 u8   Datalink_close=0;  //挂断后不再登陆
 u8     Current_UDP_sd=0;   // 及时上报 标志位 
 u8   ReadFail_sd_current=0;  // ReadFail Send current



u8  COPS_Couter=0;             // COPS  返回次数

u8  CSQ_counter=0;
u8  CSQ_Duration=32;    //查询CSQ 的定时间隔 
u8  CSQ_flag=1;
u8  ModuleSQ=0;  //GSM 模块信号强度数值
u8	ModuleStatus= 0;   //网络状态 

u8  Light=0;
u8   Mocule_Data_Come=0; // 模块收到数据 

u8   Send_DataFlag=0;  // 发送GSM data Flag
u8   Receive_DataFlag=0;// 接收数据

#ifdef 0
#ifdef SMS_ENABLE
     SMS   SMS_Service;    //  短息相关     
#endif 


/*
    应用相关函数
*/

#ifdef  SMS_ENABLE
void  SMS_timer_lnw(void)
{
	 //-------- 短信相关 ------------------
	 if(SMS_Service.SMS_come==1)
	 {
		 SMS_Service.SMS_delayCounter++;
		 if(SMS_Service.SMS_delayCounter>0) 
		   {
			 SMS_Service.SMS_delayCounter=0; 
			 SMS_Service.SMS_come=0;
			 SMS_Service.SMS_read=1;	  // 使能读取
		   }
	 }

	if(SMS_Service.SMS_sendFlag==2)
	{
              SMS_Service.SMS_delayCounter++;
		 if(SMS_Service.SMS_delayCounter>1) 
		   {
			 SMS_Service.SMS_delayCounter=0; 
			 SMS_Service.SMS_come=0;
			 SMS_Service.SMS_sendFlag=3;  // 使能读取
		   }
	}
}
void  SMS_timer(void)
{
	if(SMS_Service.SMS_waitCounter)
		SMS_Service.SMS_waitCounter--;
	if(SMS_Service.SMS_delALL>1)
		{
		SMS_Service.SMS_delALL--;
		}
	 //-------- 短信相关 ------------------
	 /*
	 if(SMS_Service.SMS_come==1)
	 {
		 SMS_Service.SMS_delayCounter++;
		 if(SMS_Service.SMS_delayCounter)
		   {
			 SMS_Service.SMS_delayCounter=0;
			 SMS_Service.SMS_come=0;
			 SMS_Service.SMS_read=3;	  // 使能读取
		   }
	 }
	 */
}

void  SMS_Process(void)
{
	u16   	ContentLen=0;
	u16 		i,j,k;
	char *pstrTemp;
	if(SMS_Service.SMS_waitCounter)
		return;
	//-----------  短信处理相关 -------------------------------------------------------- 
	//---------------------------------
	if(SMS_Service.SMS_read)	   // 读取短信
	{
		memset(SMS_Service.SMSAtSend,0,sizeof(SMS_Service.SMSAtSend));
		/*
		strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGR=" );	  
		if ( SMS_Service.SMIndex > 9 )
		{
			SMS_Service.SMSAtSend[8] = ( SMS_Service.SMIndex >> 4 ) + 0x30;
			SMS_Service.SMSAtSend[9] = ( SMS_Service.SMIndex & 0x0f ) + 0x30;
			SMS_Service.SMSAtSend[10] = 0x0d;
			SMS_Service.SMSAtSend[11] = 0x0a;
		}
		else
		{
			SMS_Service.SMSAtSend[8] = ( SMS_Service.SMIndex & 0x0f ) + 0x30;
			SMS_Service.SMSAtSend[9] = 0x0d;
			SMS_Service.SMSAtSend[10] = 0x0a;
		}
		rt_kprintf("\r\n%s",SMS_Service.SMSAtSend); 
		*/
		///
		sprintf(SMS_Service.SMSAtSend,"AT+CMGR=%d\r\n",SMS_Service.SMIndex);
		rt_kprintf("%s",SMS_Service.SMSAtSend); 
		///
		rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );   		
		SMS_Service.SMS_read--;  
		SMS_Service.SMS_waitCounter=3;
	}
	//-------------------------------
	//       发送短息确认
	else if(SMS_Service.SMS_sendFlag==1)
	{
		#ifdef SMS_TYPE_PDU
		
		memset(SMS_Service.SMSAtSend,0,sizeof(SMS_Service.SMSAtSend));
		///申请600字节空间
		pstrTemp=rt_malloc(600);
		memset(pstrTemp,0,600);
		///将字符串格式的目的电话号码设置为PDU格式的号码
		SetPhoneNumToPDU(SMS_Service.Sms_Info.TPA, SMS_Service.SMS_destNum, sizeof(SMS_Service.Sms_Info.TPA));
		///生成PDU格式短信内容
		ContentLen=AnySmsEncode_NoCenter(SMS_Service.Sms_Info.TPA,GSM_UCS2,SMS_Service.SMS_sd_Content,strlen(SMS_Service.SMS_sd_Content),pstrTemp);
		//ContentLen=strlen(pstrTemp);
		///添加短信尾部标记"esc"
		pstrTemp[ContentLen]=0x1A;      // message  end  	
		//////
		sprintf( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=%d\r\n", (ContentLen-2)/2); 
		rt_kprintf("%s",SMS_Service.SMSAtSend); 
		rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );
		rt_thread_delay(50);
		//////	
		//rt_kprintf("%s",pstrTemp); 
		rt_device_write( &dev_vuart, 0, pstrTemp, strlen(pstrTemp) );
		rt_hw_gsm_output_Data( ( char * ) pstrTemp, ContentLen+1); 
		rt_free( pstrTemp );
		pstrTemp=RT_NULL;
		#else
		memset(SMS_Service.SMSAtSend,0,sizeof(SMS_Service.SMSAtSend));
		strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=\"" ); 
		//strcat(SMS_Service.SMSAtSend,"8613820554863");// Debug
		strcat(SMS_Service.SMSAtSend,SMS_Service.SMS_destNum);
		strcat(SMS_Service.SMSAtSend,"\"\r\n");

		rt_kprintf("\r\n%s",SMS_Service.SMSAtSend); 
		rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );

		rt_thread_delay(50);
		ContentLen=strlen(SMS_Service.SMS_sd_Content);
		SMS_Service.SMS_sd_Content	[ContentLen]=0x1A;      // message  end  		
		rt_kprintf("%s",SMS_Service.SMS_sd_Content); 
		rt_hw_gsm_output_Data( ( char * ) SMS_Service.SMS_sd_Content, ContentLen+1);  
		#endif
		SMS_Service.SMS_sendFlag=0;  // clear 
		SMS_Service.SMS_waitCounter=3;
	} 
	else if(SMS_Service.SMS_delALL==1)	  //删除短信
	{	   
		memset(SMS_Service.SMSAtSend,0,sizeof(SMS_Service.SMSAtSend));
		/*
		strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGD=" );	   
		if ( SMS_Service.SMIndex > 9 )
		{
			SMS_Service.SMSAtSend[8] = ( SMS_Service.SMIndex >> 4 ) + 0x30;
			SMS_Service.SMSAtSend[9] = ( SMS_Service.SMIndex & 0x0f ) + 0x30;
			SMS_Service.SMSAtSend[10] = 0x0d;
			SMS_Service.SMSAtSend[11] = 0x0a;
		}
		else
		{
			SMS_Service.SMSAtSend[8] = ( SMS_Service.SMIndex & 0x0f ) + 0x30;
			SMS_Service.SMSAtSend[9] = 0x0d;
			SMS_Service.SMSAtSend[10] = 0x0a;
		}
		rt_kprintf("\r\n%s",SMS_Service.SMSAtSend);
		*/
		///
		//sprintf(SMS_Service.SMSAtSend,"AT+CMGD=%d\r\n",SMS_Service.SMIndex);
		sprintf(SMS_Service.SMSAtSend,"AT+CMGD=0,4\r\n",SMS_Service.SMIndex);
		rt_kprintf("%s",SMS_Service.SMSAtSend); 
		///
		rt_hw_gsm_output( ( char * )SMS_Service.SMSAtSend ); 
		SMS_Service.SMS_delALL=0; 
		SMS_Service.SMS_waitCounter=3;
	}
}


///增加发送短信区域的内容，并置位发送短息标记，成功返回true，失败返回false
u8 Add_SMS_Ack_Content(char * instr,u8 ACKflag)
{
    if(ACKflag==0)
		 return false;  
     
	if(strlen(instr)+strlen(SMS_Service.SMS_sd_Content) < sizeof(SMS_Service.SMS_sd_Content))
		{
	 	strcat((char *)SMS_Service.SMS_sd_Content,instr);
	 	SMS_Service.SMS_sendFlag=1;
		return true;
		}
	return false;
}


void   SMS_protocol (u8 *instr,u16 len, u8  ACKstate)   //  ACKstate 
{
	char	sms_content[60];		///短信命令区"()"之间的内容
	char	sms_ack_data[60];		///短信每个命令包括'#'符的完整内容
	u8	u8TempBuf[6];
	u16	i=0,j=0;
	u16  cmdLen,u16Temp;
	char *p_Instr;
	char *pstrTemp,*pstrTempStart,*pstrTempEnd;

	//SYSID		///修改该值，保存flash
	///应答短信包头部分
	memset(SMS_Service.SMS_sd_Content,0,sizeof(SMS_Service.SMS_sd_Content));
	strcpy(SMS_Service.SMS_sd_Content,JT808Conf_struct.Vechicle_Info.Vech_Num);
	strcat(SMS_Service.SMS_sd_Content,"#");// Debug
	strcat(SMS_Service.SMS_sd_Content,SIM_CardID_JT808);// Debug
	/*************************处理信息****************************/
	p_Instr=(char *)instr;
	for(i=0;i<len;i++)
		{
		pstrTemp=strchr(p_Instr,'#');					///查找命令是否存在
		//instr++;
		if(pstrTemp)
			{
			p_Instr=pstrTemp+1;
			pstrTempStart=strchr((char *)pstrTemp,'(');			///查找命令内容开始位置
			pstrTempEnd=strchr((char *)pstrTemp,')');			///查找命令内容结束位置
			if((NULL==pstrTempStart)||(NULL==pstrTempEnd))
				{
				break;
				}
			rt_kprintf("\r\n短信命令格式有效 !");
			///获取命令内容
			memset(sms_ack_data,0,sizeof(sms_ack_data));
			memcpy(sms_ack_data,pstrTemp,pstrTempEnd-pstrTemp+1);

			///获取命令参数区域的内容以及参数长度
			pstrTempStart++;
			pstrTemp++;
			cmdLen=pstrTempEnd-pstrTempStart;
			memset(sms_content,0,sizeof(sms_content));
			rt_memcpy(sms_content,pstrTempStart,cmdLen);

			///进行命令匹配
			if(strncmp(pstrTemp,"DNSR",4)==0)				///  1. 设置域名
				{
				if(cmdLen<=sizeof(DomainNameStr))
					{
					if(pstrTemp[4]=='1')		///主域名
						{
						rt_kprintf("\r\n设置主域名 !");
						memset(DomainNameStr,0,sizeof(DomainNameStr));					  
						memset(SysConf_struct.DNSR,0,sizeof(DomainNameStr));
						memcpy(DomainNameStr,(char*)pstrTempStart,cmdLen);
						memcpy(SysConf_struct.DNSR,(char*)pstrTempStart,cmdLen);
						Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
						//----- 传给 GSM 模块------
						DataLink_DNSR_Set(SysConf_struct.DNSR,1); 
						
						///
						Add_SMS_Ack_Content(sms_ack_data,ACKstate);

						//------- add on 2013-6-6
						if(ACKstate==SMS_ACK_none)
						     SD_ACKflag.f_CentreCMDack_0001H=2 ;//DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断  

						}
					else if(pstrTemp[4]=='2')	///备用域名
						{
						rt_kprintf("\r\n设置备用域名 !");
						memset(DomainNameStr_aux,0,sizeof(DomainNameStr_aux));					  
						memset(SysConf_struct.DNSR_Aux,0,sizeof(DomainNameStr_aux));
						memcpy(DomainNameStr_aux,(char*)pstrTempStart,cmdLen);
						memcpy(SysConf_struct.DNSR_Aux,(char*)pstrTempStart,cmdLen);
						Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
						//----- 传给 GSM 模块------
						DataLink_DNSR2_Set(SysConf_struct.DNSR_Aux,1);
						
						///
						Add_SMS_Ack_Content(sms_ack_data,ACKstate);
						}
					else
						{
						continue;
						}
					}
				}
			else if(strncmp(pstrTemp,"PORT",4)==0)			///2. 设置端口
				{
				j=sscanf(sms_content,"%u",&u16Temp);
				if(j)
					{
					if(pstrTemp[4]=='1')		///主端口
						{
						rt_kprintf("\r\n设置主端口=%d!",u16Temp);
						RemotePort_main=u16Temp;
						SysConf_struct.Port_main=u16Temp;
						Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
						//----- 传给 GSM 模块------
						DataLink_MainSocket_set(RemoteIP_main,RemotePort_main,1);
						///
						Add_SMS_Ack_Content(sms_ack_data,ACKstate);

						//------- add on 2013-6-6
						if(ACKstate==SMS_ACK_none)
						     SD_ACKflag.f_CentreCMDack_0001H=2 ;//DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断  

						}
					else if(pstrTemp[4]=='2')	///备用端口
						{
						rt_kprintf("\r\n设置备用端口=%d!",u16Temp);
						RemotePort_aux=u16Temp;
						SysConf_struct.Port_Aux=u16Temp;
						Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
						//----- 传给 GSM 模块------
						DataLink_AuxSocket_set(RemoteIP_aux,RemotePort_aux,1);
						///
						Add_SMS_Ack_Content(sms_ack_data,ACKstate);
						}
					else
						{
						continue;
						}
					}
				
				}
			else if(strncmp(pstrTemp,"DUR",3)==0)				///3. 修改发送间隔
				{
				j=sscanf(sms_content,"%u",&u16Temp);
				if(j)
					{
					
					rt_kprintf("\r\n修改发送间隔! %d",u16Temp);
					dur(sms_content);
					/*
					JT808Conf_struct.DURATION.Default_Dur=u16Temp;
					Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
					*/

					///
					Add_SMS_Ack_Content(sms_ack_data,ACKstate);
					}
				}
			else if(strncmp(pstrTemp,"DEVICEID",8)==0)			///4. 修改终端ID
				{
				if(cmdLen<=sizeof(DeviceNumberID))
					{
					rt_kprintf("\r\n修改终端ID  !");
					memset(DeviceNumberID,0,sizeof(DeviceNumberID));
					memcpy(DeviceNumberID,pstrTempStart,cmdLen);
					DF_WriteFlashSector(DF_DeviceID_offset,0,DeviceNumberID,13); 
					///
					Add_SMS_Ack_Content(sms_ack_data,ACKstate);
					}
				else
					{
					       continue;
					}
				}    //SIM_CardID_JT808
			      else if(strncmp(pstrTemp,"SIMID",5)==0)			///4. 修改终端ID
				{
				if(cmdLen<=sizeof(DeviceNumberID))
					{
					
					memset(SIM_CardID_JT808,0,sizeof(SIM_CardID_JT808));
					memcpy(SIM_CardID_JT808,pstrTempStart,cmdLen);
					rt_kprintf("\r\n修改SIMID  !%s,%s,%d",SIM_CardID_JT808,pstrTempStart,cmdLen);
					simid(SIM_CardID_JT808);
					/*
					DF_WriteFlashSector(DF_SIMID_offset,0,SIM_CardID_JT808,13); 
					///*/
					Add_SMS_Ack_Content(sms_ack_data,ACKstate);

					//------- add on 2013-6-6
					if(ACKstate==SMS_ACK_none)
					       SD_ACKflag.f_CentreCMDack_0001H=2 ;//DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断  

					}
				else
					{
					       continue;
					}
				}
			
			else if(strncmp(pstrTemp,"IP",2)==0)				///5.设置IP地址
				{
				j = sscanf(sms_content, "%u.%u.%u.%u", (u32*)&u8TempBuf[0], (u32*)&u8TempBuf[1], (u32*)&u8TempBuf[2], (u32*)&u8TempBuf[3]);
				//j=str2ip(sms_content, u8TempBuf);
				if(j==4)
				 	{
				 	rt_kprintf("\r\n设置IP地址!");
					if(pstrTemp[2]=='1')
						{ 
						memcpy(SysConf_struct.IP_Main,u8TempBuf,4);
						memcpy(RemoteIP_main,u8TempBuf,4);
						SysConf_struct.Port_main=RemotePort_main;
						Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
						rt_kprintf("\r\n短信设置主服务器 IP: %d.%d.%d.%d : %d ",RemoteIP_main[0],RemoteIP_main[1],RemoteIP_main[2],RemoteIP_main[3],RemotePort_main);
						//-----------  Below add by Nathan  ----------------------------
						DataLink_MainSocket_set(RemoteIP_main,RemotePort_main,1);
						///
						Add_SMS_Ack_Content(sms_ack_data,ACKstate);

					     //------- add on 2013-6-6
						if(ACKstate==SMS_ACK_none)
						      SD_ACKflag.f_CentreCMDack_0001H=2 ;//DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断  

						}
					else if(pstrTemp[2]=='2')
						{
						memcpy(SysConf_struct.IP_Aux,u8TempBuf,4);
						memcpy(RemoteIP_aux,u8TempBuf,4);
						SysConf_struct.Port_Aux=RemotePort_aux;
						Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
						rt_kprintf("\r\n短信设置备用服务器 IP: %d.%d.%d.%d : %d ",RemoteIP_aux[0],RemoteIP_aux[1],RemoteIP_aux[2],RemoteIP_aux[3],RemotePort_aux);   
						//-----------  Below add by Nathan  ----------------------------
						DataLink_AuxSocket_set(RemoteIP_aux,RemotePort_aux,1);
						///
						Add_SMS_Ack_Content(sms_ack_data,ACKstate);
						}
					}
				}
			else if(strncmp(pstrTemp,"MODE",4)==0)			///6. 设置定位模式
				{
				        if(strncmp(sms_content,"BD",2)==0)
				        {
                                                 gps_mode("1");
				        }
					  if(strncmp(sms_content,"GP",2)==0)
				        {
                                                 gps_mode("2");
				        }	
					   if(strncmp(sms_content,"GN",2)==0)
				        {
                                                 gps_mode("3");  
				        }
					   Add_SMS_Ack_Content(sms_ack_data,ACKstate);
				}
			else if(strncmp(pstrTemp,"VIN",3)==0)				///7.设置车辆VIN
				{
				     vin_set(sms_content);
				    Add_SMS_Ack_Content(sms_ack_data,ACKstate);
				}
			else if(strncmp(pstrTemp,"TIREDCLEAR",10)==0)		///8.清除疲劳驾驶记录
				{
				      TiredDrv_write=0;
				      TiredDrv_read=0;	   
				      DF_Write_RecordAdd(TiredDrv_write,TiredDrv_read,TYPE_TiredDrvAdd);     
				      Add_SMS_Ack_Content(sms_ack_data,ACKstate);	  
				}
			else if(strncmp(pstrTemp,"DISCLEAR",8)==0)			///9清除里程
				{
				   	  JT808Conf_struct.DayStartDistance_32=0;
					  JT808Conf_struct.Distance_m_u32=0;
                                     Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct)); 
					   Add_SMS_Ack_Content(sms_ack_data,ACKstate);				 
				}
			else if(strncmp(pstrTemp,"RESET",5)==0)			///10.终端重启
				{
				      reset();
				}
			else if(strncmp(pstrTemp,"RELAY",5)==0)			///11.继电器控制
				{
				       if(sms_content[0]=='0')
				              debug_relay("0");
				       if(sms_content[0]=='1')
					   	debug_relay("1"); 

					Add_SMS_Ack_Content(sms_ack_data,ACKstate);	    
				}
			else if(strncmp(pstrTemp,"TAKE",4)==0)				//12./拍照
				{
				    switch(sms_content[0])
				   {
                                    case '1':
                                                 takephoto("1");
							break;		
					case '2':
                                                  takephoto("2"); 
							break;		
				       case '3':
                                                takephoto("3");
							break;	
				       case '4':
                                                  takephoto("4");
							break;
				   }
				    Add_SMS_Ack_Content(sms_ack_data,ACKstate);	
				}
			else if(strncmp(pstrTemp,"PLAY",4)==0)				///13.语音播报
				{
				      TTS_Get_Data(sms_content,strlen(sms_content));
				     Add_SMS_Ack_Content(sms_ack_data,ACKstate);		  
				}
			else if(strncmp(pstrTemp,"QUERY",5)==0)			///14.车辆状态查询
				{
				    Add_SMS_Ack_Content(sms_ack_data,ACKstate);	
				}
			else if(strncmp(pstrTemp,"ISP",3)==0)				///15.远程下载IP 端口
				{
				     ;
				    Add_SMS_Ack_Content(sms_ack_data,ACKstate);		 
				}
			else if(strncmp(pstrTemp,"PLATENUM",8)==0)
				{
				    rt_kprintf("Vech_Num is %s", sms_content);
					memset((u8*)&JT808Conf_struct.Vechicle_Info.Vech_Num,0,sizeof(JT808Conf_struct.Vechicle_Info.Vech_Num));	//clear	
				    rt_memcpy(JT808Conf_struct.Vechicle_Info.Vech_Num,sms_content,strlen(sms_content));
					Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
					Add_SMS_Ack_Content(sms_ack_data,ACKstate);
				}
			else if(strncmp(pstrTemp,"COLOR",5)==0)
				{
				j=sscanf(sms_content,"%d",&u16Temp);
					if(j)
					{
						
					JT808Conf_struct.Vechicle_Info.Dev_Color=u16Temp; 
	        		rt_kprintf("\r\n 车辆颜色: %s ,%d \r\n",sms_content,JT808Conf_struct.Vechicle_Info.Dev_Color);          
	        		Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
				  	Add_SMS_Ack_Content(sms_ack_data,ACKstate);
					}
				}
			else												
				{
				;
				}
			}
		else
			{
			break;
			}
		}
}

void SMS_Test(char * s,u8 value)
{
	SMS_protocol(s,strlen(s),value);
}
FINSH_FUNCTION_EXPORT(SMS_Test, SMS_Test);
void SMS_PDU(char *s)
{
	u16 len;
	u16 i,j;
	char *pstrTemp;
	pstrTemp=(char *)rt_malloc(160);	///短信解码后的完整内容，解码后汉子为GB码
	len=GsmDecodePdu(s,strlen(s),&SMS_Service.Sms_Info,pstrTemp);
	GetPhoneNumFromPDU( SMS_Service.SMS_destNum,  SMS_Service.Sms_Info.SCA, sizeof(SMS_Service.Sms_Info.SCA));
	rt_kprintf( "\r\n  短息中心号码:%s \r\n", SMS_Service.SMS_destNum );
	GetPhoneNumFromPDU( SMS_Service.SMS_destNum,  SMS_Service.Sms_Info.TPA, sizeof(SMS_Service.Sms_Info.TPA));
	rt_kprintf( "\r\n  短息来源号码:%s \r\n", SMS_Service.SMS_destNum );
	rt_kprintf( "\r\n 短信消息:\"%s\"\r\n",pstrTemp);
	rt_free( pstrTemp );
	pstrTemp = RT_NULL;
}
FINSH_FUNCTION_EXPORT(SMS_PDU, SMS_PDU);

void SMS_PDU_SEND(char *s)
{
	u16 len;
	u16 i;
	char *pstrTemp;
	memset(SMS_Service.SMSAtSend,0,sizeof(SMS_Service.SMSAtSend));
	pstrTemp=rt_malloc(400);
	memset(pstrTemp,0,400);
	i=0;
	SetPhoneNumToPDU(SMS_Service.Sms_Info.TPA, "8615010061062", sizeof(SMS_Service.Sms_Info.TPA));
	len=AnySmsEncode_NoCenter(SMS_Service.Sms_Info.TPA,GSM_UCS2,s,strlen(s),pstrTemp);
	//len=strlen(pstrTemp);
	pstrTemp[len]=0x1A;      // message  end  	
	//////
	sprintf( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=%d\r\n", (len-2)/2); 
	rt_kprintf("%s",SMS_Service.SMSAtSend); 
	rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );
	rt_thread_delay(50);
	//////	
	rt_device_write( &dev_vuart, 0, pstrTemp, strlen(pstrTemp) );
	//rt_hw_console_output(pstrTemp);
	rt_hw_gsm_output_Data( ( char * ) pstrTemp, len+1); 
	rt_free( pstrTemp );
	pstrTemp=RT_NULL;
}
FINSH_FUNCTION_EXPORT(SMS_PDU_SEND, SMS_PDU_SEND);
#endif
#endif


void   DialLink_TimeOut_Process(void)
{
           if( DataDial.start_dial_stateFLAG==1) 
	   {
					  DataDial.start_dial_counter++;  
					  //-------- add  on  2013 4-8  -----------
					  if( DataDial.start_dial_counter==293)
					  	    DataLink_EndFlag=1;
					  //-----------------------------------
					  if(DataDial.start_dial_counter>300)   	 
					  {
						 DataDial.start_dial_counter=0; 
						 //----------  存储休眠定时器 -----------
						 rt_kprintf( "\r\n 拨号限时使能了\r\n" );	
						 RstWrite_ACConoff_counter();
						 rt_thread_delay(8); 			  
						 reset();  //  system  reset
					  }		    
	   }
	 else
		  DataDial.start_dial_counter=0;
}

void   DialLink_TimeOut_Clear(void)
{
     DataDial.start_dial_stateFLAG=0;
     DataDial.start_dial_counter=0;
}

void   DialLink_TimeOut_Enable(void)
{
     DataDial.start_dial_stateFLAG=1;
     DataDial.start_dial_counter=0;
}


u8   DataLink_Status(void)
{ 
      if(DataLink_Online)
	  	 return   DataLink_Online  ;
	else
		 return   0;

}
   void DataLinkOK_Process(void)    // 数据连接成功后做清除状态处理
   { 
       
	   DataLink_Online=1;
	   ModuleStatus |= Status_GPRS;
	 //  Retries_dialcounter=0;  // 清除允许最大拨号次数 

	 //  Connect_counter=0;      // clear  清除登陆前重拨最大允许次数  
	 #ifdef  MULTI_LINK
	          TCP2_ready_dial=1;	 
	 #endif

	   rt_kprintf("   \r\n  -------通信链路交互成功!-----  \r\n");
   }

u8  Close_DataLink(void)
{
          DataLink_EndFlag=1; 
	    return true;
}


u8   PositionSD_Enable(void)
{
      PositionInfo_sdFlag = 1;
        return true;     	
}

u8   PositionSD_Disable(void)
{
      PositionInfo_sdFlag=0;
	      return true;
}

u8   PositionSD_Status(void)
{
     if(PositionInfo_sdFlag)
	  	 return   PositionInfo_sdFlag  ;
	else
		 return   0;
}


u8   Stop_Communicate(void)
{
     Datalink_close=1;  //挂断后不再登陆
	  return true;
}


void    GSM_SD_MsgQueue_to_APP (void) 
{
          // 1.   Rx Process 
          if (Mocule_Data_Come!=1)   return;
          // 2.   send  msgqueue
          Mocule_Data_Come=0;	  
          //rt_712Rule_MsgQue_Post(&gsm2app808_mq,(u8*)GSM_HEX,GSM_HEX_len);   
          	   
}

void Gsm_rxAppData_SemRelease(u8* instr, u16 inlen, u8 link_num)
{
	/* release semaphore to let finsh thread rx data */
	//rt_sem_release(&gsmSd_Appdata_sem);
	Send_DataFlag=1;
	gsm_rx_app_infoStruct.info=instr;
	gsm_rx_app_infoStruct.len=inlen;
	gsm_rx_app_infoStruct.link_num=link_num;
}
	
//=====================================================================
/*
          Thread   Initial    ,   Device      Related 
*/
//=====================================================================

static rt_err_t   Device_GSM_init( rt_device_t dev )
{
      //    一. ---------  GSM  Hardware  initial  ---------------------
	       rt_hw_gsm_init();  
       //    三.   GSM 状态寄存器初始化
             Gsm_RegisterInit();   //  init register states    ,then  it  will  power on  the module  
	      return RT_EOK;
}

static rt_err_t Device_GSM_open( rt_device_t dev, rt_uint16_t oflag )  
{
         return RT_EOK;
}
static rt_err_t Device_GSM_close( rt_device_t dev )
{
        return RT_EOK;
}
static rt_size_t Device_GSM_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{

        return RT_EOK;
}
static rt_size_t Device_GSM_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
 {
        //     Data_Send(GPRS_info,GPRS_infoWr_Tx); 
        Data_Send((u8*)buff,(u16)count,(u8)pos); 
        return RT_EOK;
  }


/*
                   Descirption               cmd
                     APN                        0x01
                     main_socket            0x02
                     aux_socket             0x03
                     isp_socket               0x04
                     DNSR1                    0x05
                     DNSR2                    0x06
                     power_on                0x11
                     at_init                     0x12
                     dial_gprs                 0x13 
                     query_online            0x21
                     send_gprsdata         0x22
            
*/
static rt_err_t Device_GSM_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	SOCKET    *socket=(SOCKET*)arg;  
	SOCKET   reg_socket;
	
	     reg_socket= *socket;
	
     switch(cmd)     
     	{
     	        case  APN ://                        0x01
     	                           DataLink_APN_Set((u8*)arg,1);
     	                           break;
               case  main_socket://             0x02
                                  //SOCKET type arg
                                  DataLink_MainSocket_set(reg_socket.ip,reg_socket.port,1);
                                 break;
               case  aux_socket://             0x03
                                 DataLink_AuxSocket_set(reg_socket.ip,reg_socket.port,1);
                                 break;
	        case isp_socket://               0x04
	                          DataLink_IspSocket_set(reg_socket.ip,reg_socket.port,1);  
	                          break;
	        case DNSR1 ://                   0x05              
	                          DataLink_DNSR_Set((u8*)arg,1); 
	                          break;
	        case DNSR2 ://                   0x06
	                         DataLink_DNSR2_Set((u8*)arg,1);  
	                          break;
	        case power_on ://               0x11
	                          if(GPRS_GSM_PowerON())
						return 	RT_EOK;    // GSM 模块开启完毕
				     else
					     return	RT_EBUSY;  //正在上电过程中
	        case at_init://                     0x12
	                          CommAT.Total_initial=1;
				     CommAT.Initial_step=0; 
	                          break;
	        case dial_gprs ://                0x13 
	                           rt_kprintf(" Control AT_Start\r\n");   
					 CommAT.Initial_step=0; 
					 CommAT.Total_initial=0;   

					 DataDial.Dial_ON=enable;  //  进入  Data   状态
					 DataDial.Pre_Dial_flag=1;    // Convert to  DataDial State
	                          break;
	        case query_online://            0x21
	                           if(DataLink_Status())
							 return RT_EOK;  	
				      else
					  	        return RT_ERROR;
     	}

       return RT_EOK;
 }







static void gsm_thread_entry(void* parameter)  
{
    rt_size_t  res=RT_ERROR;
        //     finsh_init(&shell->parser);
	rt_kprintf("\r\n ---> gsm thread start !\r\n");
	 //	  	 
	 
	while (1)
	{
	
            // 1.  after power  on    get imsi code  	 	
              IMSIcode_Get(); 
            //  2. after get imsi   Comm   AT  initial   start 
              GSM_Module_TotalInitial();  
            // 3. Receivce & Process   Communication  Module   data ----
	       GSM_Buffer_Read_Process(); 
	       DataLink_Process();		
             //------------------------------------------------
		    if (Send_DataFlag== 1) 
               {
			   //  rt_kprintf("\r\n  gsm rx  app msgQue:   rxlen=%d  rx_content=%s\r\n",msgq_gsm.len,msgq_gsm.info); 	 
                        //  Data_Send(GPRS_info,GPRS_infoWr_Tx); 
			   res=rt_device_control(&Device_GSM, query_online, NULL);
			    if(res==RT_EOK)
			             rt_device_write(&Device_GSM, 0,( const void *)GPRS_info,(rt_size_t) GPRS_infoWr_Tx); 
			    Send_DataFlag=0;          
	
	         }    
		//---------  Step timer
		  Dial_step_Single_10ms_timer();    
		  	  //   TTS	
                TTS_Data_Play();		  
		 
                //   Get  CSQ value
	         GSM_CSQ_Query();	 
               #ifdef SMS_ENABLE
		  //   SMS  Service
		  SMS_Process();
		 #endif	   

	         rt_thread_delay(20);  	     
			   
	}
}
 


static void timeout_gsm(void *  parameter)
{     //  1 second 
  //   init  Module related
   GPRS_GSM_PowerON(); 

  //---  Data Link END  --------
   End_Datalink(); 
  //       ISP timer
   ISP_Timer();
  //       TTS timeout 
   TTS_Exception_TimeLimt();      
  //      Voice Record
   VOC_REC_process();
 //      Dial Link process
    DialLink_TimeOut_Process();
 //  CSQ
     GSM_CSQ_timeout(); 
 
 #ifdef SMS_ENABLE
 //  SMS  timer
    SMS_timer();
 #endif
 
 //    RTC get 
    time_now=Get_RTC();  
}   



/* init gsm */
void _gsm_startup(void)
{
        rt_err_t result;

      	
       //    二. RT  相关初始化
		//-------------   sem  init  ---------------------------------
		//rt_sem_init(&gsmRx_sem, "gsm", 0, 0);        
	  rt_sem_init(&gsmSd_Appdata_sem, "Sd_data", 0, 0);   
	  rt_mq_init( &mq_GSM, "mq_GSM", &GSM_rawinfo[0], 1400 - sizeof( void* ), GSM_RAWINFO_SIZE, RT_IPC_FLAG_FIFO );

       //---------  timer_gsm ----------
	         // 1. create  timer     100ms=Dur
	      timer_gsm=rt_timer_create("tim_gsm",timeout_gsm,RT_NULL,100,RT_TIMER_FLAG_PERIODIC);//| RT_TIMER_FLAG_SOFT_TIMER);  
	        //  2. start timer
	      if(timer_gsm!=RT_NULL)
	           rt_timer_start(timer_gsm);  
 
 
	result=rt_thread_init(&gsm_thread,
		"GsmThrd",
		gsm_thread_entry, RT_NULL,
		&gsm_thread_stack[0], sizeof(gsm_thread_stack),   
		Prio_GSM, 10);  

    if (result == RT_EOK)
    {
           rt_thread_startup(&gsm_thread); 
   	    rt_kprintf("\r\n Gsm2  thread initial sucess!\r\n");    // nathan add 
    }
    else
	    rt_kprintf("\r\nGsm2  thread initial fail!\r\n");    // nathan add	   
	    
	Device_GSM.type	= RT_Device_Class_Char;
	Device_GSM.init	= Device_GSM_init;
	Device_GSM.open	=  Device_GSM_open;
	Device_GSM.close	=  Device_GSM_close;
	Device_GSM.read	=  Device_GSM_read;
	Device_GSM.write	=  Device_GSM_write;
	Device_GSM.control =Device_GSM_control; 

	rt_device_register( &Device_GSM, "GsmDev", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
	rt_device_init( &Device_GSM ); 

}




