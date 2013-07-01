/*
       APP_485.C
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "spi_sd.h"
#include "Usbh_conf.h"
#include <dfs_posix.h>  
#include <finsh.h>



/*
                  �������  
*/
 _MultiTake     MultiTake;	  //  ��·����״̬λ 
 u8  SingleCamera_TakeRetry=0; // ��·����ͷ����ʱ�����Ĵ�������
 Camera_state CameraState; 
  u8  SingleCamra_TakeResualt_BD=0;   // ��·���ս��
 u8    TX_485const_Enable=0;   // ʹ�ܷ��ͱ�־λ  
 u8 	  last_package=0; // �������һ����ʶ

/*
              ���������
*/
 VOICE_DEV Dev_Voice;  







//---------------------------Photo Related Function --------------------------------------

void Init_Camera(void)
{
   CameraState.block_counter=0;  // clear   
   CameraState.status=other;    
   CameraState.create_Flag=0; 
}
void End_Camera(void)
{
  CameraState.block_counter=0;
  CameraState.status=other;
  CameraState.camera_running=0; 
  CameraState.timeoutCounter=0;
  CameraState.OperateFlag=0;
  CameraState.create_Flag=0; 
  Dev_Voice.Poll_Enble=1;//��ʼ������������ѯ 
  _485_RXstatus._485_receiveflag=IDLE_485;  
   #ifdef LCD_5inch
  DwinLCD_work_Enable();//  ʹ��5 ����
  #endif
}
u8 Start_Camera(u8  CameraNum)
{
    if(1==ISP_running_Status())    // Զ�̸��¹����в�������
		return false;
		
        //----------------------------------------------------
		Photo_TakeCMD_Update(CameraNum);
	    //---------------------------------	
		Photo_FetchCMD_Update(CameraNum);
        
		CameraState.create_Flag=0;  
        //-------------------------------------------------------
                 //if((CameraState.camera_running==0)||(Photo_sdState.photo_sending==enable))// �ڲ����䱨��ͼƬ�������û��ͼƬ����������ִ��
                  if((CameraState.camera_running==0)&&(Photo_sdState.photo_sending==0))// �ڲ����䱨��ͼƬ�������û��ͼƬ����������ִ�� 
	 	          {
				        CameraState.camera_running=1; 
					 CameraState.status=enable;
					 TX_485const_Enable=1;
					  _485_RXstatus._485_receiveflag=IDLE_485; 
					 Dev_Voice.Poll_Enble=0;//ֹͣ������������ѯ   
                                    #ifdef LCD_5inch 
					 DwinLCD_work_Disable() ;  //  ֹͣ5 ����
					 #endif
					
					 return true;
				  }  
				  else
				  {
				    CameraState.status=other; 
					return false;
				  }
				  
}

void  Camra_Take_Exception(void)
{
         if(CameraState.camera_running==1)
         {
	           if((CameraState.timeoutCounter++)>=3)  // ÿ����һ���������ֻ�е������ͳ���3��û���ز���Ϊʧ�� 
			   {  
			      //------------  Normal  Process  --------------
			      //FeedDog;
			      End_Camera();		
				  
				 // Power_485CH1_OFF;  // ��һ·485�ĵ�			�ص繤��
				  rt_kprintf("\r\n  Camera %d  Error\r\n",Camera_Number);     
				  WatchDog_Feed();
				  delay_ms(20);  // end--duration--new start	
				  WatchDog_Feed();
				  //Power_485CH1_ON;  // ��һ·485�ĵ�			�ص繤��      
                  //------------  Multi Take Process  ------------  
                  if(1==MultiTake.Taking)
                  {
                      switch(Camera_Number)  
                      	{
                           case  1:  
						   	       MultiTake.Take_retry++;
								   if(MultiTake.Take_retry>=3) 
								   	{
								   	   //-------- old process---------------
                                       MultiTake.TakeResult[0]=Take_Fail;   // ��ʾ��һ·����ͷ����ʧ��
									   //--------- new   process-------------
                                           MultiTake.Take_retry=0;
									       //----------������һ·����ͷ-----------									   
										   Camera_Number=2;  
									       //-------------------------
									       Start_Camera(Camera_Number);
								   	}
								   else
								   	{    // continue operate this  camera 
                                          Start_Camera(Camera_Number);
								   	}

							       break;
						    case  2:  
						   	       MultiTake.Take_retry++;
								   if(MultiTake.Take_retry>=3) 
								   	{
								   	   //-------- old process---------------
                                       MultiTake.TakeResult[1]=Take_Fail;   // ��ʾ��һ·����ͷ����ʧ��
									   //--------- new   process-------------
                                           MultiTake.Take_retry=0;
									       //----------������һ·����ͷ-----------									   
										   Camera_Number=3;  
									       //-------------------------
									       Start_Camera(Camera_Number);
								   	}
								   else
								   	{    // continue operate this  camera 
                                          Start_Camera(Camera_Number);
								   	}

							       break;
						    case  3:   
						   	       MultiTake.Take_retry++; 
								   if(MultiTake.Take_retry>=3) 
								   	{
								   	   //-------- old process---------------
                                       MultiTake.TakeResult[2]=Take_Fail;   // ��ʾ��һ·����ͷ����ʧ��
									   //--------- new   process-------------
                                           MultiTake.Take_retry=0;
									       //----------������һ·����ͷ-----------									   
										   Camera_Number=4;  
									       //-------------------------
									       Start_Camera(Camera_Number);
								   	}
								   else
								   	{    // continue operate this  camera 
                                          Start_Camera(Camera_Number);
								   	}

							       break;		   
						   case  4:
						   	       MultiTake.Take_retry++;
								   if(MultiTake.Take_retry>=3)
								   	{
								   	   //-------- old process---------------
                                       MultiTake.TakeResult[3]=Take_Fail;   // ��ʾ��һ·����ͷ����ʧ��
									   //--------- new   process-------------
                                           MultiTake.Take_retry=0;
									       /*
									       Taking End, Start Transfering 
									        */
									    MultiTake.Taking=0;  // Taking  State  Over									    
									    Check_MultiTakeResult_b4Trans();
								   	}
								   else
								   	{    // continue operate this  camera 
                                          Start_Camera(Camera_Number);
								   	}

							       break;			
						   default:
						   	       MultiTake_End();   // clear and  exit Mutitake state
						   	       break;
                      	}


                  }
				 else
				 {    //  ��������
                    SingleCamera_TakeRetry++;
					if(SingleCamera_TakeRetry>=3)
						{
                                                  SingleCamera_TakeRetry=0;//clear
							 rt_kprintf("\r\n     ��·���ճ���������!\r\n");  
							 SingleCamra_TakeResualt_BD=1;
						}
					else
					   	{    // continue operate this  camera 
                              Start_Camera(Camera_Number);
							  //rt_kprintf("\r\n  ������ %d \r\n",Camera_Number); 
					   	}
				 }
				  
	           }
         }




}

void MultiTake_Start(void)
{
    u8 i=0;
	
    MultiTake.Taking=1;     // enable
	MultiTake.Transfering=0;
	MultiTake.CurretCameraNum=1;
	for(i=0;i<Max_CameraNum;i++)
	   MultiTake.TakeResult[i]=Take_idle;
	MultiTake.Take_retry=0;  
	MultiTake.Take_success_counter=0;
	//----------------------------------	
        Camera_Number=1;   // �� 1  Start from  1  Camera
	    Start_Camera(Camera_Number);  // ��ʼִ������   

     //------------------
     SingleCamera_TakeRetry=0;  // add later    		
     
} 

void MultiTake_End(void) 
{
    u8 i=0;
	
    MultiTake.Taking=0;     
	MultiTake.Transfering=0;
	MultiTake.CurretCameraNum=1;
	for(i=0;i<Max_CameraNum;i++)
	   MultiTake.TakeResult[i]=Take_idle;
    MultiTake.Take_retry=0;	
	MultiTake.Take_success_counter=0;

}


u8   Camera_Take_Enable(void)
{
         //  ������״̬���У� û�ж�·���պͶ�·���������£���������
          if((CameraState.status==other)&&(0==MultiTake.Taking)&&(0==MultiTake.Transfering))
                return  true;
	   else
	   	   return false;
   }

 u8  Check_MultiTakeResult_b4Trans(void)
{ 
    u8 i=0;

	 MultiTake.Take_success_counter=0; 
     for(i=0;i<Max_CameraNum;i++)
	 {
	 	 if(Take_Success==MultiTake.TakeResult[i])
	 	 {
	 	    MultiTake.Take_success_counter++;
	 	 }
	 }
	// Read_picNum=MultiTake.Take_success_counter-1; // 0  ������һ��      
	 if(MultiTake.Take_success_counter)
	 {
	 	    MultiTake.Transfering=1;  // Set Transfering 
	 	    MultiTake.Take_success_counter--;  // ��ʼ�ϴ��͵ü� 1
	 	    //----------- �������ճɹ�������ͷ���  �ϱ�����Ҫ���-----------
	 	     for(i=0;i<Max_CameraNum;i++) 
			 {
				 	 if(Take_Success==MultiTake.TakeResult[i])
				 	 { 
				 	    Camera_Number=i+1;
						MultiTake.TakeResult[i]=Take_idle;// clear state
						break;
				 	 }
	         }
			rt_kprintf("\r\n            ��·��ʼ Camera=%d  \r\n",Camera_Number);  
			Photo_send_start(Camera_Number);   //��ʼ׼������      
			rt_kprintf("\r\n            ��ý���¼��ϴ�MediaObj.SD_media_Flag=%d , MediaObj.SD_Eventstate=%d  \r\n",MediaObj.SD_media_Flag,MediaObj.SD_Eventstate);  
			return  1;
	 }					 
	 else
	 {
	    MultiTake_End();
		return  0;
	 }	

}

void Send_const485(u8  flag) 
{

  if(!TX_485const_Enable)
  	return;
 
  switch(CameraState.status)
  	{
       case enable:
                     rt_device_write(&Device_485,0,(char const*)Take_photo, 10);  // ������������ 
                    // rt_hw_485_Output_Data(Take_photo, 10);  // ������������ 
                    CameraState.OperateFlag=1;
					CameraState.create_Flag=1; // ��Ҫ������ͼƬ�ļ� 
                    last_package=0; 
					 rt_kprintf("\r\n  Camera: %d  ������������\r\n",Camera_Number);
					 CameraState.timeoutCounter=0; // send clear
				    break; 
	   case  transfer:
	             	//----------_485_content_wr=0;
				  _485_RXstatus._485_receiveflag=IDLE_485;  
					// rt_kprintf("\r\n ����ͼƬ��ȡ����\r\n"); 
                                  rt_device_write(&Device_485,0,(char const*)Fectch_photo, 10); // ����ȡͼ����
                            //   rt_hw_485_Output_Data(Fectch_photo, 10);  // ������������   
                               CameraState.OperateFlag=1;
				   CameraState.timeoutCounter=0;// sendclear
					//rt_kprintf("\r\n  ����ȡͼ����\r\n");					
		            break;
	   case  other	:
	   	
	   	            break;
	    default:    
	   	            break;
  	}  
   TX_485const_Enable=0;  // clear     
}
 
void takephoto(u8* str) 
{
  
  if (strlen((const char*)str)==0)
  {
	 
	  rt_kprintf("\r\n ������Ҫ���յ�����ͷ���\r\n");	  
	  return ;
  }
  else 
  {  
     Camera_Number=(str[0]-0x30);
     rt_kprintf("\r\n CameraNum=  %d\r\n",Camera_Number);	   
     Start_Camera(Camera_Number);    
	 return ;  
  }
} 

FINSH_FUNCTION_EXPORT(takephoto,Take photo); 


int str2ipport(char *buf, u8 *ip, u16 *port)
{	// convert an ip:port string into a binary values
	int	i;
	u16	_ip[4], _port;
	

	_port = 0;
	memset(_ip, 0, sizeof(_ip));

	strtrim((u8*)buf, ' ');
   
	i = sscanf(buf, "%u.%u.%u.%u:%u", (u32*)&_ip[0], (u32*)&_ip[1], (u32*)&_ip[2], (u32*)&_ip[3], (u32*)&_port);

	*(u8*)(ip + 0) = (u8)_ip[0];
	*(u8*)(ip + 1) = (u8)_ip[1];
	*(u8*)(ip + 2) = (u8)_ip[2];
	*(u8*)(ip + 3) = (u8)_ip[3];
	*port = _port;

	return i;
}


//-------------------------------------------------------------------------






