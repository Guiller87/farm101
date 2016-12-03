
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "printf_mod.h"
#include "main.h"
#include "xbee_test.h"

extern SemaphoreHandle_t xSemaphore_printf;
extern QueueHandle_t xQueue_parse_xbee_rx;

extern uint8_t xbee_rx_buff[XBEE_RX_BUF_SIZE];

//void xbee_send(unit8_t * d
//{
//		uint8_t* buffer;
//		buffer = buf;
//}


void xbee_rx_task(void * pvParameters) { 

		uint8_t i, checksum, temp, delimiter, ret_val = 1;
		uint16_t data_lenght= 0;
		uint8_t frame_type =0;
		uint8_t xbee_rx[XBEE_RX_BUF_SIZE];	
			
		while(1)
		{
				if(xQueueReceive(xQueue_parse_xbee_rx, &data_lenght, 1000)) 
				{
						memcpy(xbee_rx, xbee_rx_buff, (data_lenght + 3) );
						
						frame_type = xbee_rx[FRAME_TYPE];
						checksum = xbee_rx[data_lenght + 2];
						
						//check checksum
						temp = 0;
						for(i = 3; i < (data_lenght + 2); i++)					
						{
							temp += xbee_rx[i];
						}						
						temp = 0xFF - temp;
					
						if( temp != checksum )
						{
							if(xSemaphoreTake(xSemaphore_printf, 0))
							{
								printf("XBEE checksum error\r\n");
								xSemaphoreGive(xSemaphore_printf);
							}
						}
						else
						{
							if( frame_type == TX_STAT )
							{
								if( xbee_rx[DELIVER_STAT] == TX_SUCCESS )
								{
									if(xSemaphoreTake(xSemaphore_printf, 0))
									{
										printf("XBEE Transimit Success. \r\n");
										xSemaphoreGive(xSemaphore_printf);
									}										
								}
							}
							else if( frame_type == RX_PACKET )
							{
								if(xSemaphoreTake(xSemaphore_printf, 0))
								{
									printf("XBEE Packet Recieved:  ");
									HAL_UART_Transmit(&huart2, &xbee_rx[RF_DATA], (data_lenght - (SRC_ADDR_16 + 1)), 1000);
									printf("\r\n");
									xSemaphoreGive(xSemaphore_printf);
								}
							}
							else
							{
								if(xSemaphoreTake(xSemaphore_printf, 0))
								{
									printf("XBEE error code = %d_%d\r\n", FRAME_TYPE, xbee_rx[FRAME_TYPE]);
									xSemaphoreGive(xSemaphore_printf);
								}		
							}
						}
				}
	 }		
			
}