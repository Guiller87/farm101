
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
#include "servo_test.h"

extern uint8_t xbee_flag;
extern uint8_t sys_stat;

extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim4;
extern QueueHandle_t xQueue_test_task;

extern QueueHandle_t xQueue_rtc_alarm;
extern QueueHandle_t xQueue_water_garden;
extern QueueHandle_t xQueue_check_flow_meters;
extern QueueHandle_t xQueue_read_temp_humid;
extern QueueHandle_t xQueue_read_soil_moisture;
extern QueueHandle_t xQueue_read_garden_tank_level;
extern QueueHandle_t xQueue_water_towers;
extern QueueHandle_t xQueue_check_fish_food;
extern QueueHandle_t xQueue_check_fish_tank_level;
extern QueueHandle_t xQueue_feed_fish;

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

			/********feed fish start ******************/ //3 times a day
			// use else if with flow meters checking. they are not to be done at the same time.
			if( ((time.Hours == AQUA_FEED_FISH_HOUR_1) && (time.Minutes == AQUA_FEED_FISH_MIN_1)) ||  //9AM
					((time.Hours == AQUA_FEED_FISH_HOUR_2) && (time.Minutes == AQUA_FEED_FISH_MIN_2)) ||  //5PM
					((time.Hours == AQUA_FEED_FISH_HOUR_3) && (time.Minutes == AQUA_FEED_FISH_MIN_3)) )     //1AM
			{
			  xQueueSend(xQueue_feed_fish, &buf, 1000);
			}
			/********feed fish end******************/
			
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
	HAL_GPIO_WritePin(XBEE_RESET_GPIO_Port, XBEE_RESET_Pin, GPIO_PIN_SET);	
}



//test task only. do not run in final
void test_task(void * pvParameters)
{
	uint32_t ret_val, ret_val1, ret_val2, ret_val3, ret_val4 = 0;
	uint32_t data = 0;
	uint16_t humidity = 0;
	uint16_t temperature = 0;	
	uint32_t chan = 0;
	uint8_t buf = 146;
	uint8_t xbee_tx_buf[95] = {0x7E, 0x00, 0x5B, 0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0xCB, 0x9A, 0xFF, 0xFE, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 
															 0x67, 0x65, 0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x45, 0x44, 0x2E, 0x20, 0x66, 0x61, 0x72, 0x6D, 0x69, 0x6E, 0x67, 0x20, 0x31, 0x30, 0x31, 0x20, 0x69, 0x73, 0x20, 0x69, 0x6E, 0x20, 0x64, 0x65,
															 0x76, 0x65, 0x6C, 0x70, 0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x20, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x78, 0x62, 0x65, 0x65, 0x20, 0x6D, 0x6F, 0x64, 0x75, 0x6C, 0x65, 0x21, 0x9C};

	uint8_t test_data[5];

	test_data[0] = 0x4D;
	test_data[1] = 0x61;
	test_data[2] = 0x6F;
	test_data[3] = 0x30;

	TIM_OC_InitTypeDef sConfigOC;
	
	float unit_per_deg;
	

  //sConfigOC.Pulse = 0x1388; //2.5ms
		 sConfigOC.Pulse = 0x7D0; //
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	
	HAL_TIM_Base_Stop(&htim4);
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
	
  HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3);
	
	HAL_TIM_Base_Start(&htim4);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	
	while(1)
	{
	//	if(xQueueReceive(xQueue_test_task, &buf, 1000))
//		{		
		
			//test pumps and flow meters
//			pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, OFF);
//			HAL_Delay(200);
//			start_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);
//			pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, ON);
//			HAL_Delay(2000);
//			ret_val = read_flow_rate(GARDEN_PUMP_FLOW_METER_Pin);			
//			pump_switch(GARDEN_PUMP_GPIO_Port, GARDEN_PUMP_Pin, OFF);			
//			printf("GARDEN Flow meter reading = %d\r\n", ret_val);
//			
//			pump_switch(AQUA_PUMP_TOWER_GPIO_Port, AQUA_PUMP_TOWER_Pin, OFF);			
//			HAL_Delay(200);
//			start_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);
//			pump_switch(AQUA_PUMP_TOWER_GPIO_Port, AQUA_PUMP_TOWER_Pin, ON);
//			HAL_Delay(2000);
//			ret_val = read_flow_rate(AQUA_PUMP_TOWER_FLOW_METER_Pin);			
//			pump_switch(AQUA_PUMP_TOWER_GPIO_Port, AQUA_PUMP_TOWER_Pin, OFF);
//			printf("AQUA_PUMP_TOWER meter reading = %d\r\n", ret_val);

//			pump_switch(AQUA_PUMP_FISH_TANK_GPIO_Port, AQUA_PUMP_FISH_TANK_Pin, OFF);			
//			HAL_Delay(200);
//			start_flow_rate(AQUA_PUMP_FISH_TANK_FLOW_METER_Pin);
//			pump_switch(AQUA_PUMP_FISH_TANK_GPIO_Port, AQUA_PUMP_FISH_TANK_Pin, ON);
//			HAL_Delay(2000);
//			ret_val = read_flow_rate(AQUA_PUMP_FISH_TANK_FLOW_METER_Pin);			
//			pump_switch(AQUA_PUMP_FISH_TANK_GPIO_Port, AQUA_PUMP_FISH_TANK_Pin, OFF);
//			printf("AQUA_PUMP_FISH_TANK meter reading = %d\r\n", ret_val);
				
//			ret_val1 = read_soil_moisture(&hadc1, ADC_CHANNEL_6, 4);
//			ret_val2 = read_soil_moisture(&hadc1, ADC_CHANNEL_13, 4);
//			ret_val3 = read_soil_moisture(&hadc1, ADC_CHANNEL_12, 4);
//			ret_val4 = read_soil_moisture(&hadc1, ADC_CHANNEL_9, 4);
//			ret_val1 /= 10;
//			ret_val2 /= 10;
//			ret_val3 /= 10;
//			ret_val4 /= 10;
//			printf("Soil moisture reading: Sensor 1 = %d, Sensor 2 = %d, Sensor 3 = %d, Sensor 4 = %d \r\n", ret_val1, ret_val2, ret_val3, ret_val4);
//							

//				ret_val = measure_distance(GARDEN_TANK_LEVEL_TRIG_GPIO_Port, GARDEN_TANK_LEVEL_TRIG_Pin, 
//																	 GARDEN_TANK_LEVEL_ECHO_GPIO_Port, GARDEN_TANK_LEVEL_ECHO_Pin);
//				printf("GARDEN tank water level = %d cm\r\n", ret_val);
//        HAL_Delay(2000);
//				ret_val = measure_distance(AQUA_FISH_TANK_LEVEL_TRIG_GPIO_Port, AQUA_FISH_TANK_LEVEL_TRIG_Pin, 
//																	 AQUA_FISH_TANK_LEVEL_ECHO_GPIO_Port, AQUA_FISH_TANK_LEVEL_ECHO_Pin);
//        printf("AQUA Fish tank level = %d cm\r\n", ret_val);
//        HAL_Delay(2000);
//				ret_val = measure_distance(AQUA_FISH_FOOD_LEVEL_TRGI_GPIO_Port, AQUA_FISH_FOOD_LEVEL_TRGI_Pin, 
//																	 AQUA_FISH_FOOD_LEVEL_ECHO_GPIO_Port, AQUA_FISH_FOOD_LEVEL_ECHO_Pin);
//        printf("AQUA Fish food level = %d cm\r\n", ret_val);
//        HAL_Delay(2000);


			for(ret_val = 0x7D0; ret_val < 0x1388; (ret_val += 16))
			{
				sConfigOC.Pulse = ret_val; //
				sConfigOC.OCMode = TIM_OCMODE_PWM1;
				sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
				sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
				
				HAL_TIM_Base_Stop(&htim4);
				HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
				
				HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3);
				
				HAL_TIM_Base_Start(&htim4);
				HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
				HAL_Delay(10);
			}
			
			for(ret_val = 0x1388; ret_val > 0x7D0; ret_val -= 16)
			{
				sConfigOC.Pulse = ret_val; //
				sConfigOC.OCMode = TIM_OCMODE_PWM1;
				sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
				sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
				
				HAL_TIM_Base_Stop(&htim4);
				HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
				
				HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3);
				
				HAL_TIM_Base_Start(&htim4);
				HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
				HAL_Delay(10);
			} 

//		}
  }
}

