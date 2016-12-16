
#include "main.h"
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "printf_mod.h"
#include "main.h"
#include "xbee_test.h"
#include "aqua_tasks.h"
#include "util_func.h"
#include "water_pump.h"
#include "water_flow.h"
#include "ultrasonic.h"

extern uint32_t sys_stat;
extern uint8_t xbee_flag;
extern uint8_t base_stn_dest_addr[];
extern UART_HandleTypeDef huart5;

extern QueueHandle_t xQueue_water_towers;
extern QueueHandle_t xQueue_check_fish_food;
extern QueueHandle_t xQueue_check_fish_tank_level;

extern SemaphoreHandle_t xSemaphore_send_log;
extern SemaphoreHandle_t xSemaphore_timer2;

void water_towers(void * pvParameters)
{
	uint8_t count, buf;
	uint32_t ret_val;
	uint8_t value[3] = {0};
	
	while(1)
	{
		if(xQueueReceive(xQueue_water_towers, &ret_val, 1000))
		{	
      sys_stat |= SYS_STAT_AQUA_WATERING_MASK;
			
			if(xSemaphoreTake(xSemaphore_send_log, 10000))
			{
				printf("AQUA Watering Towers START\r\n");
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Watering Towers START", 26, &buf, 0);
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}
				xSemaphoreGive(xSemaphore_send_log);
			}
			
			start_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);

			pump_switch(AQUA_PUMP_TOWER_GPIO_Port, AQUA_PUMP_TOWER_Pin, ON);
			//water for 60s
			vTaskDelay(60000 / portTICK_PERIOD_MS );
			pump_switch(AQUA_PUMP_TOWER_GPIO_Port, AQUA_PUMP_TOWER_Pin, OFF);
			
			ret_val = read_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);
			
			//update status
			if(xSemaphoreTake(xSemaphore_send_log, 10000))
			{
				printf("AQUA Watering Towers END");
				printf("AQUA Towers Flow meter reading = %d\r\n", ret_val);

				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Watering Towers END", 24, &buf, 0);
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
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Towers Flow meter reading = ", 33, &value[0], 3);
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
			start_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);
			vTaskDelay(10000 / portTICK_PERIOD_MS );
			ret_val = read_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);
			
			//update status
			if(xSemaphoreTake(xSemaphore_send_log, 10000))
			{
				if(ret_val > 3) 
				{
				  printf("AQUA Tower Watering Pump failed to turned off!!!");

					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Tower Watering Pump failed to turned off!!!", 48, &value[0], 0);
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
					printf("AQUA Tower Watering Pump turned off...");

					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Tower Watering Pump turned off", 35, &value[0], 0);
							vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
							if(xbee_flag & TX_OK) 
							{
								xbee_flag &= (~TX_OK);
								break;
							}
					}	
				}	
				printf("AQUA Towers Flow meter reading = %d\r\n", ret_val);
				
				convert_to_ascii(ret_val, &value[0]);	
				for(count = 0; count < MAX_RETRY; count++)
				{
						xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Towers Flow meter reading = ", 33, &value[0], 3);				
						vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
						if(xbee_flag & TX_OK) 
						{
							xbee_flag &= (~TX_OK);
							break;
						}
				}	
				xSemaphoreGive(xSemaphore_send_log);						
			}

      sys_stat &= (~SYS_STAT_AQUA_WATERING_MASK);
		}
	}
}

void check_fish_food(void * pvParameters)
{
	uint8_t count, buf;
	uint32_t ret_val;
	uint8_t value[3] = {0};
	
	while(1)
	{
		if(xQueueReceive(xQueue_check_fish_food, &ret_val, 1000))
		{	
      sys_stat |= SYS_STAT_AQUA_CHECKING_FISH_FOOD;
			
			if(xSemaphoreTake(xSemaphore_timer2, 1000))
			{			
				vTaskSuspendAll();
				ret_val = measure_distance(GARDEN_TANK_LEVEL_TRIG_GPIO_Port, GARDEN_TANK_LEVEL_TRIG_Pin, 
																	 GARDEN_TANK_LEVEL_ECHO_GPIO_Port, GARDEN_TANK_LEVEL_ECHO_Pin);
				xTaskResumeAll();
				
				xSemaphoreGive(xSemaphore_timer2);
			}
			
			if(xSemaphoreTake(xSemaphore_send_log, 10000))
			{
				if(ret_val == DISTANCE_ERROR)
				{
					printf("AQUA Fish food level check failed\r\n");

					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Fish food level check failed", 33, &value[0], 0);
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
					printf("AQUA Fish food level = %d cm\r\n", ret_val);
					
					convert_to_ascii(ret_val, &value[0]);
					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Fish food level in cm = ", 29, &value[0], 3);
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
			
      sys_stat &= (~SYS_STAT_AQUA_CHECKING_FISH_FOOD);			
		}
	}
}

void check_fish_tank_level(void * pvParameters)
{
	uint8_t count, buf;
	uint32_t ret_val;
	uint8_t value[3] = {0};
	
	while(1)
	{
		if(xQueueReceive(xQueue_check_fish_tank_level, &ret_val, 1000))
		{	
      sys_stat |= SYS_STAT_AQUA_CHECKING_FISH_TANK;
			
			if(xSemaphoreTake(xSemaphore_timer2, 1000))
			{			
				vTaskSuspendAll();
				ret_val = measure_distance(GARDEN_TANK_LEVEL_TRIG_GPIO_Port, GARDEN_TANK_LEVEL_TRIG_Pin, 
																	 GARDEN_TANK_LEVEL_ECHO_GPIO_Port, GARDEN_TANK_LEVEL_ECHO_Pin);
				xTaskResumeAll();
				
				xSemaphoreGive(xSemaphore_timer2);
			}
				
				
			if(xSemaphoreTake(xSemaphore_send_log, 10000))
			{
				if(ret_val == DISTANCE_ERROR)
				{
					printf("AQUA Fish tank level check failed\r\n");

					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Fish tank level check failed", 33, &value[0], 0);
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
					printf("AQUA Fish tank level = %d cm\r\n", ret_val);
					
					convert_to_ascii(ret_val, &value[0]);
					for(count = 0; count < MAX_RETRY; count++)
					{
							xbee_tx(&huart5, &base_stn_dest_addr[0] , "AQUA Fish tank level in cm = ", 29, &value[0], 3);
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
			
			sys_stat &= (~SYS_STAT_AQUA_CHECKING_FISH_TANK);
		}
	}
}

