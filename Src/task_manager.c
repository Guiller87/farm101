
#include "main.h"
#include "task_manager.h"
#include "task_schedules.h"
#include "cmsis_os.h"
#include "util_func.h"
#include "water_pump.h"
#include "water_flow.h"


extern QueueHandle_t xQueue_rtc_alarm;
extern QueueHandle_t xQueue_water_garden;
extern QueueHandle_t xQueue_check_flow_meters;

extern RTC_HandleTypeDef hrtc;
extern SemaphoreHandle_t xSemaphore_printf;


//used in test_task for testing only
extern TIM_HandleTypeDef htim3; //timer used for garden pump flow sensor


void manage_garden( void * pvParameters )
{
		uint8_t buf = 0;
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
					    ((time.Hours == GARDEN_PUMP_SCHED_HOUR_3) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_3))     //1AM
					  )
					{
							 xQueueSend(xQueue_water_garden, &buf, 1000);
					} /********garden pump watering end******************/
					/********flow meters checking start ******************/	
					//set to every 10 minutes by checking in every multiple of 10 minutes
					 else if( (time.Minutes == 0) || (time.Minutes == 10) || (time.Minutes == 20) || (time.Minutes == 30) ||
		          (time.Minutes == 40) || (time.Minutes == 50) || (time.Minutes == 60)
					)
					{
					     xQueueSend(xQueue_check_flow_meters, &buf, 1000);
					}
					/********flow meters checking end ******************/			
			
					
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
	
	
	
		while(1)
		{
          start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
			    vTaskDelay(3000 / portTICK_PERIOD_MS );
			    ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
			
					if(xSemaphoreTake(xSemaphore_printf, 5000))
					{
							printf("flow rate = %d \r\n", ret_val);
							xSemaphoreGive(xSemaphore_printf);
					}				
					
		}
}

