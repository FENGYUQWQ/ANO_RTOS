#include "Drv_UP_flow.h"
#include "Drv_Uart.h"

#include "FreeRTOS.h"
#include "task.h"

#define CIRCLE_P(n,a,b) ((a) + ((n)-(a))%((b)-(a)))
#define OF_BUFFER_NUM 14

uint8_t of_init_type;
uint8_t of_init_cnt;
uint8_t of_buf_update_cnt;
uint8_t OF_DATA[OF_BUFFER_NUM];
static uint8_t com_getdata[OF_BUFFER_NUM]= {0};//buffer

const static uint8_t Sensor_cfg[]={
0x12, 0x80, 0x11, 0x30, 0x1b, 0x06, 0x6b, 0x43, 0x12, 0x20, 
0x3a, 0x00, 0x15, 0x02, 0x62, 0x81, 0x08, 0xa0, 0x06, 0x68, 
0x2b, 0x20, 0x92, 0x25, 0x27, 0x97, 0x17, 0x01, 0x18, 0x79,
0x19, 0x00, 0x1a, 0xa0, 0x03, 0x00, 0x13, 0x00, 0x01, 0x13, 
0x02, 0x20, 0x87, 0x16, 0x8c, 0x01, 0x8d, 0xcc, 0x13, 0x07, 
0x33, 0x10, 0x34, 0x1d, 0x35, 0x46, 0x36, 0x40, 0x37, 0xa4,
0x38, 0x7c, 0x65, 0x46, 0x66, 0x46, 0x6e, 0x20, 0x9b, 0xa4, 
0x9c, 0x7c, 0xbc, 0x0c, 0xbd, 0xa4, 0xbe, 0x7c, 0x20, 0x09, 
0x09, 0x03, 0x72, 0x2f, 0x73, 0x2f, 0x74, 0xa7, 0x75, 0x12,
0x79, 0x8d, 0x7a, 0x00, 0x7e, 0xfa, 0x70, 0x0f, 0x7c, 0x84, 
0x7d, 0xba, 0x5b, 0xc2, 0x76, 0x90, 0x7b, 0x55, 0x71, 0x46, 
0x77, 0xdd, 0x13, 0x0f, 0x8a, 0x10, 0x8b, 0x20, 0x8e, 0x21,
0x8f, 0x40, 0x94, 0x41, 0x95, 0x7e, 0x96, 0x7f, 0x97, 0xf3, 
0x13, 0x07, 0x24, 0x58, 0x97, 0x48, 0x25, 0x08, 0x94, 0xb5, 
0x95, 0xc0, 0x80, 0xf4, 0x81, 0xe0, 0x82, 0x1b, 0x83, 0x37,
0x84, 0x39, 0x85, 0x58, 0x86, 0xff, 0x89, 0x15, 0x8a, 0xb8, 
0x8b, 0x99, 0x39, 0x98, 0x3f, 0x98, 0x90, 0xa0, 0x91, 0xe0, 
0x40, 0x20, 0x41, 0x28, 0x42, 0x26, 0x43, 0x25, 0x44, 0x1f,
0x45, 0x1a, 0x46, 0x16, 0x47, 0x12, 0x48, 0x0f, 0x49, 0x0d, 
0x4b, 0x0b, 0x4c, 0x0a, 0x4e, 0x08, 0x4f, 0x06, 0x50, 0x06, 
0x5a, 0x56, 0x51, 0x1b, 0x52, 0x04, 0x53, 0x4a, 0x54, 0x26,
0x57, 0x75, 0x58, 0x2b, 0x5a, 0xd6, 0x51, 0x28, 0x52, 0x1e, 
0x53, 0x9e, 0x54, 0x70, 0x57, 0x50, 0x58, 0x07, 0x5c, 0x28, 
0xb0, 0xe0, 0xb1, 0xc0, 0xb2, 0xb0, 0xb3, 0x4f, 0xb4, 0x63,
0xb4, 0xe3, 0xb1, 0xf0, 0xb2, 0xa0, 0x55, 0x00, 0x56, 0x40, 
0x96, 0x50, 0x9a, 0x30, 0x6a, 0x81, 0x23, 0x33, 0xa0, 0xd0, 
0xa1, 0x31, 0xa6, 0x04, 0xa2, 0x0f, 0xa3, 0x2b, 0xa4, 0x0f,
0xa5, 0x2b, 0xa7, 0x9a, 0xa8, 0x1c, 0xa9, 0x11, 0xaa, 0x16, 
0xab, 0x16, 0xac, 0x3c, 0xad, 0xf0, 0xae, 0x57, 0xc6, 0xaa, 
0xd2, 0x78, 0xd0, 0xb4, 0xd1, 0x00, 0xc8, 0x10, 0xc9, 0x12,
0xd3, 0x09, 0xd4, 0x2a, 0xee, 0x4c, 0x7e, 0xfa, 0x74, 0xa7, 
0x78, 0x4e, 0x60, 0xe7, 0x61, 0xc8, 0x6d, 0x70, 0x1e, 0x39, 
0x98, 0x1a, 0x9d, 0xf0
};

u8 Drv_OFInit(void)
{
	uint8_t send_buffer[10];
	uint8_t i;
	//rst
	of_init_cnt = 0;
	
	//
	send_buffer[0] = 0xAA;
	Drv_Uart4SendBuf(send_buffer, 1);

	send_buffer[0] = 0xAB;
	send_buffer[1] = 0x96;
	send_buffer[2] = 0x26;
	send_buffer[3] = 0xbc;
	send_buffer[4] = 0x50;
	send_buffer[5] = 0x5c;
	Drv_Uart4SendBuf(send_buffer, 6);

	//check_ack
	ROM_SysCtlDelay(80000 * 10 / 3);
//	vTaskDelay(10);
	//
	if (of_init_cnt >= 3)
	{
		if (com_getdata[0] == 0xab && com_getdata[1] == 0x00 && com_getdata[2] == 0xab)
		{
			of_init_cnt = 0;
		}
		else
		{
			of_init_cnt = 10;
		}
	}
	else
	{
		of_init_cnt = 10;
	}
		
	//
	uint8_t len = 161;
	for(i=0; i<len;i+=2 )
	{
		send_buffer[0] = 0xBB;	 
		send_buffer[1] = 0xdc;	
		send_buffer[2] = Sensor_cfg[i];
		send_buffer[3] = Sensor_cfg[i+1];
		send_buffer[4] = (0xdc^send_buffer[2]^send_buffer[3]);
		Drv_Uart4SendBuf(send_buffer, 5);

		//check_ack
		ROM_SysCtlDelay(80000 * 10 / 3);
//		vTaskDelay(10);
		//
		if(of_init_cnt >= 3)
		{
			if (com_getdata[0] == 0xbb && com_getdata[1] == 0x00 && com_getdata[2] == 0xbb)
			{
				of_init_cnt = 0;

			}
			else
			{
				of_init_cnt = 10;

			}
		}
		else
		{
			of_init_cnt = 10;
		}
				
	}
	
	send_buffer[0] = 0xDD;
	//
	Drv_Uart4SendBuf(send_buffer, 1);

	
	
	if(of_init_cnt == 10)
	{
		//光流初始化检测
//		while(1)
//		{
//				for(u8 i = 0;i<2;i++)
//				{
//					led_on(LED_ALL);
//					//
//					delay_ms(200);
//					//
//					led_off(LED_ALL);
//					//
//					delay_ms(200);
//				}
//				//
//				delay_ms(1500);
//				//			
//		}
		return (0);
	}
	else
	{
		//
		of_init_cnt = 255;
		//
//		of_init_type = 2;
		return (1);
	}
}


void OFGetByte(uint8_t data)
{
	static uint8_t in_p=0,head_p = 0;
	
	//
	if(of_init_cnt!=255 )
	{
		if(of_init_cnt<OF_BUFFER_NUM)
		{
			com_getdata[of_init_cnt++] = data;
		}
	}
	else	
	{
		//		
		com_getdata[in_p++] = data;
		in_p %= OF_BUFFER_NUM;

		if(in_p == head_p)
		{
			if(
						 (com_getdata[CIRCLE_P(head_p   ,0,OF_BUFFER_NUM)] == 0xFE)
					&& (com_getdata[CIRCLE_P(head_p+1 ,0,OF_BUFFER_NUM)] == 0x0A)
					&& (com_getdata[CIRCLE_P(head_p+13,0,OF_BUFFER_NUM)] == 0x55)
				)
			{
				for(uint8_t i=0; i<OF_BUFFER_NUM; i++)
				{
					OF_DATA[i] = com_getdata[CIRCLE_P(head_p+i,0,OF_BUFFER_NUM)];
				}
				of_buf_update_cnt++;

			}
			else
			{		
				head_p ++;
				head_p %= OF_BUFFER_NUM;
			}
		}
	}
}


void up_flow_loop(void *pvParameters)
{
	Drv_Uart4Init(19200);		//接优象光流
	
	Drv_Uart5Init(9600);		//接KS103超声波
	
	of_init_type = (Drv_OFInit()==0)?0:2;
	if(of_init_type==2)//优像光流初始化失败
	{
		printf("初始化优象光流成功，波特率19200\r\n");
	}
	else
	{
		Drv_Uart4Init(921600);	//接匿名光流
		Drv_Uart5Init(921600);	//openmv
		printf("初始化匿名光流，波特率921600\r\n");
	}
	
	vTaskDelete(NULL);
}
