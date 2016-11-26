
#include "main.h"
#include "cmsis_os.h"
#include "water_pump.h"
#include "water_flow.h"
#include "garden_tasks.h"
#include "util_func.h"


extern TIM_HandleTypeDef htim3; //timer used for garden pump flow sensor

extern QueueHandle_t xQueue_water_garden;
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
					vTaskDelay(30000 / portTICK_PERIOD_MS );
					pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, OFF);
					
					ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
					
					if(xSemaphoreTake(xSemaphore_printf, 5000))
					{
							printf("GARDEN Watering END...");
							printf("Flow meter reading = %d\r\n", ret_val);
							xSemaphoreGive(xSemaphore_printf);
					}
			}
		}			
}