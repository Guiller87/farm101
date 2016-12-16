
#include "main.h"
#include "task_manager.h"
#include "task_schedules.h"
#include "cmsis_os.h"
#include "util_func.h"
#include "water_pump.h"
#include "water_flow.h"
#include "am2302.h"
#include "soil_moisture.h"
#include "xbee_test.h"
#include "ultrasonic.h"

extern uint8_t xbee_flag;
extern uint8_t sys_stat;

extern UART_HandleTypeDef huart5;

extern QueueHandle_t xQueue_rtc_alarm;
extern QueueHandle_t xQueue_water_garden;
extern QueueHandle_t xQueue_check_flow_meters;
extern QueueHandle_t xQueue_read_temp_humid;
extern QueueHandle_t xQueue_read_soil_moisture;
extern QueueHandle_t xQueue_read_garden_tank_level;
extern QueueHandle_t xQueue_water_towers;
extern QueueHandle_t xQueue_check_fish_food;
extern QueueHandle_t xQueue_check_fish_tank_level;

extern RTC_HandleTypeDef hrtc;
extern ADC_HandleTypeDef hadc1;
extern SemaphoreHandle_t xSemaphore_send_log;

//used in test_task for testing only
extern TIM_HandleTypeDef htim3; //timer used for garden pump flow sensor


void manage_garden( void * pvParameters )
{
	uint8_t buf, channel, count = 0;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	
	while(1)
	{
		if(xQueueReceive(xQueue_rtc_alarm, &buf, 1000))
		{
			print_timestamp(); //remove this in final. must not print always for good logging.

			if(xSemaphoreTake(xSemaphore_send_log, 1000))
			{
				printf("sys_stat = %08x \r\n", sys_stat);
				xSemaphoreGive(xSemaphore_send_log);
			}
			
			for(count = 0; count < MAX_RETRY; count++)
			{
		      xbee_send_timestamp();
					vTaskDelay(MAX_WAIT_FOR_RESP / portTICK_PERIOD_MS );
				  if(xbee_flag & TX_OK) 
					{
						xbee_flag &= (~TX_OK);
						break;
					}
			}
			
			//get current time
			HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
		
			
			//Garden tasks
			/********garden pump watering start ******************/ //3 times a day
			// use else if with flow meters checking. they are not to be done at the same time.
			if( ((time.Hours == GARDEN_PUMP_SCHED_HOUR_1) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_1)) ||  //9AM
					((time.Hours == GARDEN_PUMP_SCHED_HOUR_2) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_2)) ||  //5PM
					((time.Hours == GARDEN_PUMP_SCHED_HOUR_3) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_3)) )     //1AM
			{
			  xQueueSend(xQueue_water_garden, &buf, 1000);
			}
			/********garden pump watering end******************/
				
			/********read ambient temperature and humitdy start******************/	//every 10 mins
			//set to every 10 minutes by checking in every multiple of 10 minutes
			if( (time.Minutes == 00) || (time.Minutes == 10) || (time.Minutes == 20) || 
					(time.Minutes == 30) || (time.Minutes == 40) || (time.Minutes == 50) )
			{
				xQueueSend(xQueue_read_temp_humid, &buf, 1000);
				xQueueSend(xQueue_read_soil_moisture, &buf, 1000);
				xQueueSend(xQueue_read_garden_tank_level, &buf, 1000);
			}					
			/********read ambient temperature and humitdy start******************/	
			
			//Aqua tasks
			/********water towers start******************/	 //every 30 mins	
			if( (time.Minutes == AQUA_WATER_TOWER_MIN_1) || (time.Minutes == AQUA_WATER_TOWER_MIN_2) )
			{
				xQueueSend(xQueue_water_towers, &buf, 1000);
			}	
			/********water towers end******************/						

			/********check fish food level start******************/		//every 1 day
			if( (time.Hours == AQUA_CHECK_FISH_FOOD_HOUR) && (time.Minutes == AQUA_CHECK_FISH_FOOD_MIN) )
			{
				xQueueSend(xQueue_check_fish_food, &buf, 1000);
			}
			/********check fish food level end******************/							
			
		  /********check fish tank level start******************/		//every 1 hour	
			if(time.Minutes == AQUA_CHECK_FISH_TANK_LVL_MIN)
			{
				xQueueSend(xQueue_check_fish_tank_level, &buf, 1000);
			}
			/********check fish tank level end******************/					
			
      //common tasks
			/********flow meters checking start ******************/	 //every 10 mins
			//set to every 10 minutes by checking in every multiple of 10 minutes
			if( (time.Minutes == 00) || (time.Minutes == 10) || (time.Minutes == 20) ||
			  (time.Minutes == 30) || (time.Minutes == 40) || (time.Minutes == 50) )
			{
				if( ((time.Hours == GARDEN_PUMP_SCHED_HOUR_1) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_1)) ||  //9AM
						((time.Hours == GARDEN_PUMP_SCHED_HOUR_2) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_2)) ||  //5PM
						((time.Hours == GARDEN_PUMP_SCHED_HOUR_3) && (time.Minutes == GARDEN_PUMP_SCHED_MIN_3)) )     //1AM
				{
					//if it is also time to water plant in garden, do nothing
				}
				else if( (time.Minutes == AQUA_WATER_TOWER_MIN_1) || (time.Minutes == AQUA_WATER_TOWER_MIN_2) )
				{
					//if it is also time to water towers, do nothing
				}	
				else
				{
					xQueueSend(xQueue_check_flow_meters, &buf, 1000);
				}
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
	pump_switch(AQUA_PUMP_FISH_TANK_GPIO_Port, AQUA_PUMP_FISH_TANK_Pin, ON);	
}



//test task only. do not run in final
void test_task(void * pvParameters)
{
	uint32_t ret_val = 0;
	uint32_t data = 0;
	uint16_t humidity = 0;
	uint16_t temperature = 0;	
	uint32_t chan = 0;
	uint8_t buf = 146;
	uint8_t xbee_tx_buf[95] = {0x7E, 0x00, 0x5B, 0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0xCB, 0x9A, 0xFF, 0xFE, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 
															 0x67, 0x65, 0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x45, 0x44, 0x2E, 0x20, 0x66, 0x61, 0x72, 0x6D, 0x69, 0x6E, 0x67, 0x20, 0x31, 0x30, 0x31, 0x20, 0x69, 0x73, 0x20, 0x69, 0x6E, 0x20, 0x64, 0x65,
															 0x76, 0x65, 0x6C, 0x70, 0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x20, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x78, 0x62, 0x65, 0x65, 0x20, 0x6D, 0x6F, 0x64, 0x75, 0x6C, 0x65, 0x21, 0x9C};
	uint8_t dest_addr[8];
	dest_addr[0] = 0x00;
	dest_addr[1] = 0x13;
	dest_addr[2] = 0xA2;
	dest_addr[3] = 0x00;
	dest_addr[4] = 0x40;
	dest_addr[5] = 0xE7;
	dest_addr[6] = 0xCB;
	dest_addr[7] = 0x9A;		

	uint8_t test_data[5];

	test_data[0] = 0x4D;
	test_data[1] = 0x61;
	test_data[2] = 0x6F;
	test_data[3] = 0x30;

		
	while(1)
	{
		
//			vTaskSuspendAll();
//			ret_val = measure_distance(GARDEN_TANK_LEVEL_TRIG_GPIO_Port, GARDEN_TANK_LEVEL_TRIG_Pin, 
//																 GARDEN_TANK_LEVEL_ECHO_GPIO_Port, GARDEN_TANK_LEVEL_ECHO_Pin);
//			xTaskResumeAll();

//			if(xSemaphoreTake(xSemaphore_send_log, 5000))
//			{
//				if(ret_val == DISTANCE_ERROR)
//				{
//					printf("AQUA Fish tank level check failed\r\n");
//				}
//				else
//				{
//					printf("AQUA Fish tank level = %d cm\r\n", ret_val);
//				}
//				xSemaphoreGive(xSemaphore_send_log);
//			}		
				//xQueueSend(xQueue_read_temp_humid, &buf, 1000);
		xQueueSend(xQueue_read_soil_moisture, &buf, 1000);
			vTaskDelay(3000 / portTICK_PERIOD_MS );
		
//		HAL_GPIO_TogglePin(GARDEN_TANK_LEVEL_TRIG_GPIO_Port, GARDEN_TANK_LEVEL_TRIG_Pin);
//		timer_delay_uS(80000);
//			
		    //HAL_UART_Transmit(&huart5, xbee_tx_buf, 95, 1000);
//		for( data=0; data < 10; data++){
//			print_timestamp();
//			xbee_send_timestamp();
//			vTaskDelay(3000 / portTICK_PERIOD_MS );
//		}
//						if(xSemaphoreTake(xSemaphore_send_log, 5000))
//						{
//								printf("start");
//								xSemaphoreGive(xSemaphore_send_log);
//						}						
//			
//					  //check aqua towers watering status
//						start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
//						vTaskDelay(10000 / portTICK_PERIOD_MS );  //monitor for 10 seconds
//						ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
//						//update status
//						if(xSemaphoreTake(xSemaphore_send_log, 5000))
//						{
//								printf("Aqua towers flow meter reading = %d\r\n", ret_val);
//								xSemaphoreGive(xSemaphore_send_log);
//						}			
//						chan = ADC_CHANNEL_6;
//						//ret_val = read_soil_moisture(hadc1, chan);
//						if(xSemaphoreTake(xSemaphore_send_log, 5000))
//						{
//								printf("ADC Channel%d = %d\r\n", chan,ret_val );
//								xSemaphoreGive(xSemaphore_send_log);
//						}		
//		xQueueSend(xQueue_read_garden_tank_level, &buf, 1000);
//		vTaskDelay(3000 / portTICK_PERIOD_MS );
	} 
}

