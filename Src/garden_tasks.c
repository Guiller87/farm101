
#include "main.h"
#include "cmsis_os.h"
#include "water_pump.h"
#include "garden_tasks.h"
#include "util_func.h"



extern QueueHandle_t xQueue_water_garden;
extern SemaphoreHandle_t xSemaphore_printf;

void water_garden(void * pvParameters)
{
		uint8_t buf = 0;
	
		while(1)
		{
			if(xQueueReceive(xQueue_water_garden, &buf, 1000))
			{
					print_timestamp();
					if(xSemaphoreTake(xSemaphore_printf, 5000))
					{
							printf("GARDEN Watering START...");
							xSemaphoreGive(xSemaphore_printf);
					}
					
					pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, ON);
					vTaskDelay(30000 / portTICK_PERIOD_MS );
					pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, OFF);
					
					if(xSemaphoreTake(xSemaphore_printf, 5000))
					{
							printf("GARDEN Watering END...");
							xSemaphoreGive(xSemaphore_printf);
					}					
			}
		}			
}