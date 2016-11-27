
#include "main.h"
#include "cmsis_os.h"
#include "water_pump.h"
#include "water_flow.h"
#include "garden_tasks.h"
#include "util_func.h"

extern QueueHandle_t xQueue_water_garden;
extern QueueHandle_t xQueue_check_flow_meters;

extern SemaphoreHandle_t xSemaphore_printf;

void water_garden(void * pvParameters)
{
		uint8_t buf = 0;
	  uint32_t ret_val = 0;
		while(1)
		{
				if(xQueueReceive(xQueue_water_garden, &buf, 1000))
				{
						print_timestamp();
						if(xSemaphoreTake(xSemaphore_printf, 5000))
						{
								printf("GARDEN Watering START... \r\n");
								xSemaphoreGive(xSemaphore_printf);
						}
						
						start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
			
						pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, ON);
						//water for 30s
						vTaskDelay(30000 / portTICK_PERIOD_MS );
						pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, OFF);
						
						ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
						
						//update status
						if(xSemaphoreTake(xSemaphore_printf, 5000))
						{
								printf("GARDEN Watering END...");
								printf("Flow meter reading = %d\r\n", ret_val);
								xSemaphoreGive(xSemaphore_printf);
						}
						
						//wait for 10s to drain remaining water in the line
						vTaskDelay(10000 / portTICK_PERIOD_MS );
						
						//check if valve is successfully turned off
						start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
						vTaskDelay(10000 / portTICK_PERIOD_MS );
						ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
						
						//update status
						if(xSemaphoreTake(xSemaphore_printf, 5000))
						{
								if(ret_val > 3) 
								{
										printf("GARDEN Watering Pump failed to turned off!!!");
								} 		
								else 
								{
										printf("GARDEN Watering Pump successfully turned off...");
								}	
								printf("Flow meter reading = %d\r\n", ret_val);
								xSemaphoreGive(xSemaphore_printf);						
						}
				}
		}			
}

void check_flow_meters(void * pvParameters)
{
		uint8_t buf = 0;
	  uint32_t ret_val = 0;
		while(1)
		{
				if(xQueueReceive(xQueue_check_flow_meters, &buf, 1000))
				{	 
					  //check garden watering status
						start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
						vTaskDelay(10000 / portTICK_PERIOD_MS );  //monitor for 10 seconds
						ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
						//update status
						if(xSemaphoreTake(xSemaphore_printf, 5000))
						{
								printf("Garden flow meter reading = %d\r\n", ret_val);
								xSemaphoreGive(xSemaphore_printf);
						}
						
					  //check aqua towers watering status
						start_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);
						vTaskDelay(10000 / portTICK_PERIOD_MS );  //monitor for 10 seconds
						ret_val = read_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);
						//update status
						if(xSemaphoreTake(xSemaphore_printf, 5000))
						{
								printf("Aqua towers flow meter reading = %d\r\n", ret_val);
								xSemaphoreGive(xSemaphore_printf);
						}

					  //check aqua fish tank water flow status
						start_flow_rate(AQUA_PUMP_FISH_TANK_FLOW_METER_Pin);
						vTaskDelay(10000 / portTICK_PERIOD_MS );  //monitor for 10 seconds
						ret_val = read_flow_rate(AQUA_PUMP_FISH_TANK_FLOW_METER_Pin);
						//update status
						if(xSemaphoreTake(xSemaphore_printf, 5000))
						{
								printf("Aqua fishtank flow meter reading = %d\r\n", ret_val);
							  printf("Reading as of ");
								xSemaphoreGive(xSemaphore_printf);
						}
						print_timestamp();
						
				}				
		}
	
}
