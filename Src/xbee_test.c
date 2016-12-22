
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "printf_mod.h"
#include "main.h"
#include "xbee_test.h"
#include "util_func.h"

extern UART_HandleTypeDef huart5;
extern RTC_HandleTypeDef hrtc;

extern SemaphoreHandle_t xSemaphore_send_log;
extern QueueHandle_t xQueue_parse_xbee_rx;

extern uint8_t xbee_rx_buff[XBEE_RX_BUF_SIZE];
extern uint8_t xbee_tx_buff[XBEE_TX_BUF_SIZE];
extern uint8_t xbee_tx_buff_offset;
extern uint8_t xbee_flag;
//my xbee address 00 13 A2 00 40 E7 CB 9E

extern uint8_t base_stn_dest_addr[];


void xbee_tx(UART_HandleTypeDef * huart, uint8_t* dest_addr, char* text, uint8_t text_len, uint8_t* value, uint8_t value_len)
{
	uint8_t len = text_len + value_len;
	
	//uint8_t xbee_tx_buff[100];
	uint8_t data_lenght = len + 14; // data + overhead
	uint8_t packet_lenght = data_lenght + 3; 
	uint8_t chksum, i, temp, temp2 = 0;
	uint8_t* addr = dest_addr;
	
	xbee_tx_buff[START] = 0x7E;
	xbee_tx_buff[LENGHT] = 0;
	xbee_tx_buff[LENGHT + 1] = data_lenght;
	xbee_tx_buff[FRAME_TYPE] = TX_REQ;
	xbee_tx_buff[FRAME_ID] = 0x01;
	
	//64-bit dest addr
	for(i = 0; i < 8; i++)
	{
		xbee_tx_buff[DEST_ADDR_LONG + i] = *dest_addr++;
	}

	xbee_tx_buff[DEST_ADDR] = 0xFF;
	xbee_tx_buff[DEST_ADDR + 1] = 0xFE;
	xbee_tx_buff[BROD_RAD] = 0x00;
	xbee_tx_buff[OPTIONS] = 0x00;
	
	//tx data
	//for text
	for(i = 0; i < text_len; i++)
	{
		xbee_tx_buff[TX_DATA + i] = *text++;
	}
	//for value
	for(i = 0; i < value_len; i++)
	{
		xbee_tx_buff[TX_DATA + i + text_len] = *value++;
	}	
  //computer for checksum
	temp = 0;
	for(i = 3; i < (data_lenght + 3); i++)					
	{
		temp2 = xbee_tx_buff[i];
		temp += temp2;
	}						
	chksum = 0xFF - temp;	
	xbee_tx_buff[packet_lenght] = chksum;
	
	HAL_UART_Transmit(huart, xbee_tx_buff, (packet_lenght + 1), 1000);
}

void xbee_rx_task(void * pvParameters) 
{ 
	uint8_t i, checksum, temp, delimiter, ret_val = 1;
	uint16_t data_lenght= 0;
	uint8_t frame_type =0;
	uint8_t xbee_rx[XBEE_RX_BUF_SIZE];	
	uint8_t tx_buf[50] = {0};
	
	while(1)
	{
		if(xQueueReceive(xQueue_parse_xbee_rx, &data_lenght, 500)) 
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
					if(xSemaphoreTake(xSemaphore_send_log, 100))
					{
						printf("XBEE checksum error\r\n");
						xSemaphoreGive(xSemaphore_send_log);
					}
				}
				else
				{
					if(frame_type == TX_STAT)
					{
						if(xbee_rx[DELIVER_STAT] == TX_SUCCESS)
						{
							xbee_flag |= TX_OK; //set tx flag. very important!
							if(xSemaphoreTake(xSemaphore_send_log, 100))
							{
								printf("XBEE Transimit Success. \r\n");
								xSemaphoreGive(xSemaphore_send_log);
							}										
						}
					}
					else if(frame_type == RX_PACKET)
					{
						if(xSemaphoreTake(xSemaphore_send_log, 100))
						{
							printf("XBEE Packet Recieved:  ");
							HAL_UART_Transmit(&huart2, &xbee_rx[RF_DATA], (data_lenght - (SRC_ADDR_16 + 1)), 1000);
							printf("\r\n");
							xSemaphoreGive(xSemaphore_send_log);
						}
					}
					else if(frame_type == MODEM_STATUS)
					{
						if(xSemaphoreTake(xSemaphore_send_log, 100))
						{
							printf("XBEE Modem Status Received: %d", xbee_rx[MODEM_STATUS_VAL]);
							//HAL_UART_Transmit(&huart2, &xbee_rx[MODEM_STATUS_VAL], 1, 1000);
							printf("\r\n");
							xSemaphoreGive(xSemaphore_send_log);
						}
					}					
					else
					{
						if(xSemaphoreTake(xSemaphore_send_log, 100))
						{
							printf("XBEE frame type not supported %d\r\n", xbee_rx[FRAME_TYPE]);
							xSemaphoreGive(xSemaphore_send_log);
						}		
					}
				}
		}
	}		
}

void xbee_tx_packet_create(uint8_t* dest_addr)
{
	uint8_t* addr = dest_addr;
	uint8_t i = 0;
	
	xbee_tx_buff_offset = 0;
	
	xbee_tx_buff[START] = 0x7E;
	//xbee_tx_buff[LENGHT] = 0;
	//xbee_tx_buff[LENGHT + 1] = data_lenght;
	xbee_tx_buff[FRAME_TYPE] = TX_REQ;
	xbee_tx_buff[FRAME_ID] = 0x01;
	
	//64-bit dest addr
	for(i = 0; i < 8; i++)
	{
		xbee_tx_buff[DEST_ADDR_LONG + i] = *dest_addr++;
	}

	xbee_tx_buff[DEST_ADDR] = 0xFF;
	xbee_tx_buff[DEST_ADDR + 1] = 0xFE;
	xbee_tx_buff[BROD_RAD] = 0x00;
	xbee_tx_buff[OPTIONS] = 0x00;	
}
void xbee_tx_packet_update(char* data, uint8_t len)
{
	uint8_t i = 0;
	
	for(i = 0; i < len; i++)
	{
		xbee_tx_buff[TX_DATA + i + xbee_tx_buff_offset] = *data++;
	}
	xbee_tx_buff_offset += len;
}
void xbee_tx_packet_send(UART_HandleTypeDef * huart)
{
	uint8_t i, temp, temp2, chksum = 0;
	uint8_t data_lenght = xbee_tx_buff_offset + 14;
	uint8_t packet_lenght = data_lenght + 3; 
	
	xbee_tx_buff[LENGHT] = 0;
	xbee_tx_buff[LENGHT + 1] = data_lenght;

  //computer for checksum
	temp = 0;
	for(i = 3; i < (data_lenght + 3); i++)					
	{
		temp2 = xbee_tx_buff[i];
		temp += temp2;
	}						
	chksum = 0xFF - temp;	
	xbee_tx_buff[packet_lenght] = chksum;
	
	HAL_UART_Transmit(huart, xbee_tx_buff, (packet_lenght + 1), 1000);	
}

void xbee_send_timestamp(void) 	
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

  uint8_t value[3] = {0};

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

	if(xSemaphoreTake(xSemaphore_send_log, 1000))
	{	
		xbee_tx_packet_create(&base_stn_dest_addr[0]);
		
		xbee_tx_packet_update("TIME = ", 7);
		
		convert_to_ascii(time.Hours, &value[0]);
		xbee_tx_packet_update(&value[1], 2);
		
		xbee_tx_packet_update(":", 1);
		
		convert_to_ascii(time.Minutes, &value[0]);
		xbee_tx_packet_update(&value[1], 2);
			
		xbee_tx_packet_update(":", 1);
		
		convert_to_ascii(time.Seconds, &value[0]);
		xbee_tx_packet_update(&value[1], 2);
		
		xbee_tx_packet_send(&huart5);
		
		xSemaphoreGive(xSemaphore_send_log);
	}
}