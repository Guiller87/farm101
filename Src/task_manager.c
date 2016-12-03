
#include "main.h"
#include "task_manager.h"
#include "task_schedules.h"
#include "cmsis_os.h"
#include "util_func.h"
#include "water_pump.h"
#include "water_flow.h"
#include "am2302.h"
#include "soil_moisture.h"

extern UART_HandleTypeDef huart5;

extern QueueHandle_t xQueue_rtc_alarm;
extern QueueHandle_t xQueue_water_garden;
extern QueueHandle_t xQueue_check_flow_meters;
extern QueueHandle_t xQueue_read_temp_humid;
extern QueueHandle_t xQueue_read_soil_moisture;
extern QueueHandle_t xQueue_read_garden_tank_level;

extern RTC_HandleTypeDef hrtc;
extern ADC_HandleTypeDef hadc1;
extern SemaphoreHandle_t xSemaphore_printf;

//used in test_task for testing only
extern TIM_HandleTypeDef htim3; //timer used for garden pump flow sensor


void manage_garden( void * pvParameters )
{
	uint8_t buf, channel = 0;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	
	while(1)
	{
		if(xQueueReceive(xQueue_rtc_alarm, &buf, 1000))
		{
			print_timestamp(); //remove this in final. must not print always for good logging.
		
			//get current time
			HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
		
			/********garden pump watering start ******************/
			// use else if with flow meters checking. they are not to be done at the same time.
			if( ((time.Hours == GARDEN_PUMP_SCHED_HOUR_1) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_1)) ||  //9AM
					((time.Hours == GARDEN_PUMP_SCHED_HOUR_2) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_2)) ||  //5PM
					((time.Hours == GARDEN_PUMP_SCHED_HOUR_3) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_3)) )     //1AM
			{
			  xQueueSend(xQueue_water_garden, &buf, 1000);
			} 
			/********garden pump watering end******************/
			
			/********flow meters checking start ******************/	
			//set to every 10 minutes by checking in every multiple of 10 minutes
			else if( (time.Minutes == 00) || (time.Minutes == 10) || (time.Minutes == 20) ||
							 (time.Minutes == 30) || (time.Minutes == 40) || (time.Minutes == 50) )
			{
			  xQueueSend(xQueue_check_flow_meters, &buf, 1000);
			}
			/********flow meters checking end ******************/			
	
			/********read ambient temperature and humitdy start******************/	
			//set to every 10 minutes by checking in every multiple of 10 minutes
			if( (time.Minutes == 00) || (time.Minutes == 10) || (time.Minutes == 20) || 
					(time.Minutes == 30) || (time.Minutes == 40) || (time.Minutes == 50) )
			{
				xQueueSend(xQueue_read_temp_humid, &buf, 1000);
				xQueueSend(xQueue_read_soil_moisture, &buf, 1000);
				xQueueSend(xQueue_read_garden_tank_level, &buf, 1000);
			}					
			/********read ambient temperature and humitdy start******************/	
			
			set_RTC_alarm(SYS_RTC_ALARM_MIN);
		}
	}
}


void initialize_system (void)
{
	set_RTC_alarm(1);
	pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, OFF);
}



//test task only. do not run in final
void test_task(void * pvParameters)
{
	uint32_t ret_val = 0;
	uint32_t data = 0;
	uint16_t humidity = 0;
	uint16_t temperature = 0;	
	uint32_t chan = 0;
	uint8_t buf;
	uint8_t xbee_tx_buf[95] = {0x7E, 0x00, 0x5B, 0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0xCB, 0x9A, 0xFF, 0xFE, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 
															 0x67, 0x65, 0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x45, 0x44, 0x2E, 0x20, 0x66, 0x61, 0x72, 0x6D, 0x69, 0x6E, 0x67, 0x20, 0x31, 0x30, 0x31, 0x20, 0x69, 0x73, 0x20, 0x69, 0x6E, 0x20, 0x64, 0x65,
															 0x76, 0x65, 0x6C, 0x70, 0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x20, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x78, 0x62, 0x65, 0x65, 0x20, 0x6D, 0x6F, 0x64, 0x75, 0x6C, 0x65, 0x21, 0x9C};
		
	while(1)
	{
		   // HAL_UART_Transmit(&huart5, xbee_tx_buf, 95, 1000);
			//vTaskDelay(3000 / portTICK_PERIOD_MS );
		
//						if(xSemaphoreTake(xSemaphore_printf, 5000))
//						{
//								printf("start");
//								xSemaphoreGive(xSemaphore_printf);
//						}						
//			
//					  //check aqua towers watering status
//						start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
//						vTaskDelay(10000 / portTICK_PERIOD_MS );  //monitor for 10 seconds
//						ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
//						//update status
//						if(xSemaphoreTake(xSemaphore_printf, 5000))
//						{
//								printf("Aqua towers flow meter reading = %d\r\n", ret_val);
//								xSemaphoreGive(xSemaphore_printf);
//						}			
//						chan = ADC_CHANNEL_6;
//						//ret_val = read_soil_moisture(hadc1, chan);
//						if(xSemaphoreTake(xSemaphore_printf, 5000))
//						{
//								printf("ADC Channel%d = %d\r\n", chan,ret_val );
//								xSemaphoreGive(xSemaphore_printf);
//						}		
		xQueueSend(xQueue_read_garden_tank_level, &buf, 1000);
		vTaskDelay(3000 / portTICK_PERIOD_MS );
	} 
}

