
#include "main.h"
#include "cmsis_os.h"
#include "water_pump.h"
#include "water_flow.h"
#include "garden_tasks.h"
#include "util_func.h"
#include "am2302.h"
#include "soil_moisture.h"
#include "ultrasonic.h"
#include "xbee_test.h"

extern uint32_t sys_stat;
extern uint8_t xbee_flag;
extern uint8_t base_stn_dest_addr[];

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart5;

extern QueueHandle_t xQueue_water_garden;
extern QueueHandle_t xQueue_check_flow_meters;
extern QueueHandle_t xQueue_read_temp_humid;
extern QueueHandle_t xQueue_read_soil_moisture;
extern QueueHandle_t xQueue_read_garden_tank_level;
extern SemaphoreHandle_t xSemaphore_send_log;
extern SemaphoreHandle_t xSemaphore_timer2;


void water_garden(void * pvParameters)
{
	uint8_t buf, count = 0;
	uint8_t ret_val = 0;
	uint8_t value[4] = {0};
	
	while(1)
	{
		if(xQueueReceive(xQueue_water_garden, &buf, 1000))
		{
			sys_stat |= SYS_STAT_GARDEN_WATERING_MASK;
			
			if(xSemaphoreTake(xSemaphore_send_log, 5000))
			{
				printf("GARDEN Watering START\r\n");
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN Watering START", 21, &buf, 0);
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}
				xSemaphoreGive(xSemaphore_send_log);
			}
			
			start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);

			pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, ON);
			//water for 30s
			vTaskDelay(30000 / portTICK_PERIOD_MS );
			pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, OFF);
			
			ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
			
			//update status
			if(xSemaphoreTake(xSemaphore_send_log, 5000))
			{
				printf("GARDEN Watering END");
				printf("GARDEN Flow meter reading = %d\r\n", ret_val);

				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN Watering END", 19, &buf, 0);
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}

				convert_to_ascii(ret_val, &value[0]);
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN Flow meter reading = ", 29, &value[0], 3);
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}				
	
				xSemaphoreGive(xSemaphore_send_log);
			}
			
			//wait for 10s to drain remaining water in the line
			vTaskDelay(10000 / portTICK_PERIOD_MS );
			
			//check if valve is successfully turned off
			start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
			vTaskDelay(10000 / portTICK_PERIOD_MS );
			ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
			
			//update status
			if(xSemaphoreTake(xSemaphore_send_log, 5000))
			{
				if(ret_val > 3) 
				{
				  printf("GARDEN Watering Pump failed to turned off!!!");

					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN Watering Pump failed to turned off!!!", 44, &value[0], 0);
							vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
							if(xbee_flag & TX_OK) 
							{
								xbee_flag &= (~TX_OK);
								break;
							}
					}	
				} 		
				else 
				{
					printf("GARDEN Watering Pump turned off...");

					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN Watering Pump turned off...", 34, &value[0], 0);
							vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
							if(xbee_flag & TX_OK) 
							{
								xbee_flag &= (~TX_OK);
								break;
							}
					}	
				}	
				printf("GARDEN Flow meter reading = %d\r\n", ret_val);
				
				convert_to_ascii(ret_val, &value[0]);	
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN Flow meter reading = ", 29, &value[0], 3);				
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}	
				xSemaphoreGive(xSemaphore_send_log);						
			}
			
			sys_stat &= (~SYS_STAT_GARDEN_WATERING_MASK);
		}
	}			
}

void check_flow_meters(void * pvParameters)
{
	uint8_t buf, count = 0;
	uint32_t ret_val = 0;
	uint8_t value[4] = {0};
	while(1)
	{
		if(xQueueReceive(xQueue_check_flow_meters, &buf, 1000))
		{	 
			sys_stat |= SYS_STAT_GARDEN_CHECKING_WATERFLOW;
			
			//check garden watering status
			start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
			vTaskDelay(10000 / portTICK_PERIOD_MS );  //monitor for 10 seconds
			ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
			//update status
			if(xSemaphoreTake(xSemaphore_send_log, 5000))
			{
				printf("GARDEN flow meter reading = %d\r\n", ret_val);
				
				convert_to_ascii(ret_val, &value[0]);		
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN flow meter reading = ", 28, &value[0], 3);			
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}
				xSemaphoreGive(xSemaphore_send_log);
			}
			
			//check aqua towers watering status
			start_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);
			vTaskDelay(10000 / portTICK_PERIOD_MS );  //monitor for 10 seconds
			ret_val = read_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);
			//update status
			if(xSemaphoreTake(xSemaphore_send_log, 5000))
			{
				printf("AQUA towers flow meter reading = %d\r\n", ret_val);
				
				convert_to_ascii(ret_val, &value[0]);	
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA towers flow meter reading = ", 33, &value[0], 3);			
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}
				xSemaphoreGive(xSemaphore_send_log);
			}

			//check aqua fish tank water flow status
			start_flow_rate(AQUA_PUMP_FISH_TANK_FLOW_METER_Pin);
			vTaskDelay(10000 / portTICK_PERIOD_MS );  //monitor for 10 seconds
			ret_val = read_flow_rate(AQUA_PUMP_FISH_TANK_FLOW_METER_Pin);
			//update status
			if(xSemaphoreTake(xSemaphore_send_log, 5000))
			{
				printf("AQUA fishtank flow meter reading = %d\r\n", ret_val);
				
				convert_to_ascii(ret_val, &value[0]);
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA fishtank flow meter reading = ", 35, &value[0], 3);
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}				
				xSemaphoreGive(xSemaphore_send_log);
			}
			
			sys_stat &= (~SYS_STAT_GARDEN_CHECKING_WATERFLOW);
		}				
	}
}

void read_temp_humid(void * pvParameters)
{
	uint8_t buf, data_val, count = 0;
	uint8_t value[3] = {0};
	uint8_t ret_val = 0;
	uint32_t data = 0;
	uint16_t humidity = 0;
	uint16_t temperature = 0;

	while(1)
	{
		if(xQueueReceive(xQueue_read_temp_humid, &buf, 1000))
		{
			sys_stat |= SYS_STAT_GARDEN_CHECKING_HUMID_TEMP;
			
			if(xSemaphoreTake(xSemaphore_timer2, 3000))
			{					
				vTaskSuspendAll();
				ret_val = read_data_am2302(GARDEN_DHT22_GPIO_Port, GARDEN_DHT22_Pin, &data);
				xTaskResumeAll();
				xSemaphoreGive(xSemaphore_timer2);
			}
			
			humidity = (uint16_t) ((data & 0xFFFF0000) >> 16);
			temperature = (uint16_t) (data & 0xFFFF);
	
			if(xSemaphoreTake(xSemaphore_send_log, 3000))
			{
				if(!ret_val) {
					printf("Humidity = %d.%d %%RH    ", (humidity / 10), (humidity % 10) );

					xbee_tx_packet_create(&base_stn_dest_addr[0]);
					
					xbee_tx_packet_update("Humidity = ", 11);
					data_val = (humidity / 10);
					convert_to_ascii(data_val, &value[0]);
					xbee_tx_packet_update(&value[1], 2);
					xbee_tx_packet_update(".", 1);
					data_val = (humidity % 10);
					convert_to_ascii(data_val, &value[0]);
					xbee_tx_packet_update(&value[1], 2);					
					xbee_tx_packet_update(" RH ", 4);
										
					
					if ((temperature & 0x8000) != 0) {
						printf("-");						
					}
					printf("Temp = %d.%d C \r\n", ((temperature & 0x7fff) / 10), (temperature & 0x7fff) % 10);
					xbee_tx_packet_update("Temp = ", 7);					
					data_val = ((temperature & 0x7fff) / 10);
					convert_to_ascii(data_val, &value[0]);
					xbee_tx_packet_update(&value[1], 2);	
					xbee_tx_packet_update(".", 1);
					data_val = (temperature & 0x7fff) % 10;
					convert_to_ascii(data_val, &value[0]);
					xbee_tx_packet_update(&value[1], 2);	
					
					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx_packet_send(&huart5);
							vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
							if(xbee_flag & TX_OK) 
							{
								xbee_flag &= (~TX_OK);
								break;
							}
					}							
				}
				else {
				  printf("am2302 error %d \r\n", ret_val);
					
				  convert_to_ascii(ret_val, &value[0]);		
					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "am2302 error ", 13, &value[0], 3);
							vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
							if(xbee_flag & TX_OK) 
							{
								xbee_flag &= (~TX_OK);
								break;
							}
					}
				}
				xSemaphoreGive(xSemaphore_send_log);
			}

      sys_stat &= (~SYS_STAT_GARDEN_CHECKING_HUMID_TEMP);			
		}
	}
}

void get_soil_moisture(void * pvParameters)
{
	uint8_t channel, count = 0;
	uint32_t ret_val1, ret_val2, ret_val3, ret_val4 = 0;
	uint8_t data_val = 0;
	uint8_t value[3] = {0};
	
	//GARDEN_SOIL_MOISTURE_1 = ADC_CHANNEL_6
	//GARDEN_SOIL_MOISTURE_2 = ADC_CHANNEL_13
	//GARDEN_SOIL_MOISTURE_3 = ADC_CHANNEL_12
	//GARDEN_SOIL_MOISTURE_3 = ADC_CHANNEL_9	

	while(1)
	{
		if(xQueueReceive(xQueue_read_soil_moisture, &channel, 1000))
		{	
			sys_stat |= SYS_STAT_GARDEN_CHECKING_SOIL_MOISTURE;	
			
			ret_val1 = read_soil_moisture(hadc1, ADC_CHANNEL_6, 8);
			ret_val2 = read_soil_moisture(hadc1, ADC_CHANNEL_13, 8);
			ret_val3 = read_soil_moisture(hadc1, ADC_CHANNEL_12, 8);
			ret_val4 = read_soil_moisture(hadc1, ADC_CHANNEL_9, 8);
		    
			ret_val1 /= 10;
			ret_val2 /= 10;
			ret_val3 /= 10;
			ret_val4 /= 10;
			
			if(xSemaphoreTake(xSemaphore_send_log, 3000))
			{
				printf("Soil moisture reading: Sensor 1 = %d, Sensor 2 = %d, Sensor 3 = %d, Sensor 4 = %d \r\n", ret_val1, ret_val2, ret_val3, ret_val4);
				
				xbee_tx_packet_create(&base_stn_dest_addr[0]);
				
				xbee_tx_packet_update("Soil moisture reading: Sensor 1 = ", 34);	
				convert_to_ascii(ret_val1, &value[0]);
				xbee_tx_packet_update(&value[0], 3);	
				xbee_tx_packet_update(", Sensor 2 = ", 13);					
				convert_to_ascii(ret_val2, &value[0]);
				xbee_tx_packet_update(&value[0], 3);		
				xbee_tx_packet_update(", Sensor 3 = ", 13);					
				convert_to_ascii(ret_val3, &value[0]);
				xbee_tx_packet_update(&value[0], 3);		
				xbee_tx_packet_update(", Sensor 4 = ", 13);					
				convert_to_ascii(ret_val4, &value[0]);
				xbee_tx_packet_update(&value[0], 3);	
				
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx_packet_send(&huart5);
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}	
				
				xSemaphoreGive(xSemaphore_send_log);
			}
			
			sys_stat &= (~SYS_STAT_GARDEN_CHECKING_SOIL_MOISTURE);	
		}
	}
}

void read_garden_tank_level(void * pvParameters)
{
	uint8_t count;
	uint32_t ret_val;
	uint8_t value[3] = {0};
	
	while(1)
	{
		if(xQueueReceive(xQueue_read_garden_tank_level, &ret_val, 1000))
		{	
      sys_stat |= SYS_STAT_GARDEN_CHECKING_WATER_TANK;
			
			if(xSemaphoreTake(xSemaphore_timer2, 3000))
			{			
				vTaskSuspendAll();
				ret_val = measure_distance(GARDEN_TANK_LEVEL_TRIG_GPIO_Port, GARDEN_TANK_LEVEL_TRIG_Pin, 
																	 GARDEN_TANK_LEVEL_ECHO_GPIO_Port, GARDEN_TANK_LEVEL_ECHO_Pin);
				xTaskResumeAll();

				xSemaphoreGive(xSemaphore_timer2);
			}
			if(xSemaphoreTake(xSemaphore_send_log, 3000))
			{
				if(ret_val == DISTANCE_ERROR)
				{
					printf("GARDEN tank water level reading failed \r\n");

					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN tank water level reading failed", 38, &value[0], 0);
							vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
							if(xbee_flag & TX_OK) 
							{
								xbee_flag &= (~TX_OK);
								break;
							}
					}
				}
				else
				{
					printf("GARDEN tank water level = %d cm\r\n", ret_val);
					
					convert_to_ascii(ret_val, &value[0]);
					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "GARDEN tank water level in cm = ", 32, &value[0], 3);
							vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
							if(xbee_flag & TX_OK) 
							{
								xbee_flag &= (~TX_OK);
								break;
							}
					}						

				}
				xSemaphoreGive(xSemaphore_send_log);
			}

      sys_stat &= (~SYS_STAT_GARDEN_CHECKING_WATER_TANK);			
		}
	}	
}
