
#include "main.h"
#include "cmsis_os.h"
#include "water_pump.h"
#include "water_flow.h"
#include "garden_tasks.h"
#include "util_func.h"
#include "am2302.h"
#include "soil_moisture.h"
#include "ultrasonic.h"


extern ADC_HandleTypeDef hadc1;

extern QueueHandle_t xQueue_water_garden;
extern QueueHandle_t xQueue_check_flow_meters;
extern QueueHandle_t xQueue_read_temp_humid;
extern QueueHandle_t xQueue_read_soil_moisture;
extern QueueHandle_t xQueue_read_garden_tank_level;

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
										printf("GARDEN Watering Pump turned off...");
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

void read_temp_humid(void * pvParameters)
{
		uint8_t buf = 0;
	  uint8_t ret_val = 0;
	  uint32_t data = 0;
		uint16_t humidity = 0;
		uint16_t temperature = 0;	
	
	  while(1)
		{
				if(xQueueReceive(xQueue_read_temp_humid, &buf, 1000))
				{	 
				  vTaskSuspendAll();
					ret_val = read_data_am2302(GARDEN_DHT22_GPIO_Port, GARDEN_DHT22_Pin, &data);
					xTaskResumeAll();
			
			    humidity = (uint16_t) ((data & 0xFFFF0000) >> 16);
					temperature = (uint16_t) (data & 0xFFFF);
			
					if(xSemaphoreTake(xSemaphore_printf, 0))
					{	
						if(!ret_val) {
								printf("Humidity = %d.%d %%RH    ", (humidity / 10), (humidity % 10) );
								if ((temperature & 0x8000) != 0) {
										printf("-");
								}
								printf("Temperature = %d.%d C \r\n", ((temperature & 0x7fff) / 10), (temperature & 0x7fff) % 10);
						}
						else {
							printf("am2302 error %d \r\n", ret_val);
						}
						xSemaphoreGive(xSemaphore_printf);
					}					
				}
		}
}

void get_soil_moisture(void * pvParameters)
{
		uint8_t channel = 0;
	  uint32_t ret_val1, ret_val2, ret_val3 = 0;
    
	  //GARDEN_SOIL_MOISTURE_1 = ADC_CHANNEL_6
	  //GARDEN_SOIL_MOISTURE_2 = ADC_CHANNEL_13
	  //GARDEN_SOIL_MOISTURE_3 = ADC_CHANNEL_12
	
	  while(1)
		{
				if(xQueueReceive(xQueue_read_soil_moisture, &channel, 1000))
				{	 
			      ret_val1 = read_soil_moisture(hadc1, ADC_CHANNEL_6, 8);
					  ret_val2 = read_soil_moisture(hadc1, ADC_CHANNEL_13, 8);
					  ret_val3 = read_soil_moisture(hadc1, ADC_CHANNEL_12, 8);
					
						if(xSemaphoreTake(xSemaphore_printf, 5000))
						{
							  printf("Soil moisture reading: Sensor 1 = %d, Sensor 2 = %d, Sensor 3 = %d\r\n", ret_val1, ret_val2, ret_val3);
							  xSemaphoreGive(xSemaphore_printf);
						}
				}
		}
}

void read_garden_tank_level(void * pvParameters)
{
	uint32_t ret_val;
	while(1)
	{
		if(xQueueReceive(xQueue_read_garden_tank_level, &ret_val, 1000))
		{	 
			vTaskSuspendAll();
			ret_val = measure_distance(GARDEN_TANK_LEVEL_TRIG_GPIO_Port, GARDEN_TANK_LEVEL_TRIG_Pin, 
																 GARDEN_TANK_LEVEL_ECHO_GPIO_Port, GARDEN_TANK_LEVEL_ECHO_Pin);
			xTaskResumeAll();

			if(xSemaphoreTake(xSemaphore_printf, 5000))
			{
				if(ret_val == DISTANCE_ERROR)
				{
					printf("Garden tank water level reading failed");
					xSemaphoreGive(xSemaphore_printf);					
				}
				else
				{
					printf("Garden tank water level = %d cm\r\n", ret_val);
					xSemaphoreGive(xSemaphore_printf);
				}
			}					
		}
	}	
}
