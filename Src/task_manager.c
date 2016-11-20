
#include "main.h"
#include "task_manager.h"
#include "task_schedules.h"
#include "cmsis_os.h"
#include "util_func.h"
#include "water_pump.h"

extern QueueHandle_t xQueue_rtc_alarm;
extern QueueHandle_t xQueue_water_garden;
extern RTC_HandleTypeDef hrtc;
extern SemaphoreHandle_t xSemaphore_printf;


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
				
					HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
					HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
				
					//check garden pump schedules
					if( ((time.Hours == GARDEN_PUMP_SCHED_HOUR_1) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_1)) || 
						  ((time.Hours == GARDEN_PUMP_SCHED_HOUR_2) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_2)) ||
					    ((time.Hours == GARDEN_PUMP_SCHED_HOUR_3) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_3))
					  )
					{
							 xQueueSend(xQueue_water_garden, &buf, 1000);
					}
				
				
					
					set_RTC_alarm(SYS_RTC_ALARM_MIN);
			}
		}
}


void initialize_system (void)
{
		set_RTC_alarm(1);
	  pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, OFF);
}
