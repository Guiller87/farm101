
#include "main.h"
#include "util_func.h"
#include "cmsis_os.h"

#define DEBUG_MODE 0 // or quick alarm. alarm set to 3 seconds later

extern void MX_RTC_Init(void);
extern RTC_HandleTypeDef hrtc;
extern uint8_t sys_stat;

extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim2;

extern uint8_t debug_rx_buf[DEBUG_RX_BUF_SIZE];
extern SemaphoreHandle_t xSemaphore_send_log;
extern QueueHandle_t xQueue_debug_parse_data_rx;
extern uint8_t base_stn_dest_addr[];

void print_timestamp(void) // caller of this function should not take xSemaphore_send_log
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	
	uint8_t value[3] = {0};
	
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	if(xSemaphoreTake(xSemaphore_send_log, 10000))
	{
		printf("TIME = %d:%d:%d   ", time.Hours, time.Minutes, time.Seconds);
		printf("DATE = %d/%d/%d \r\n", date.Month, date.Date, date.Year);
		xSemaphoreGive(xSemaphore_send_log);
	}
}

void debug_parse_data_rx( void * pvParameters )
{
		uint8_t buf = 0;
		RTC_TimeTypeDef time;
		RTC_DateTypeDef date;
	  RTC_AlarmTypeDef alarm;
	
		while(1){
			
			if(xQueueReceive(xQueue_debug_parse_data_rx, &buf, 1000)) {
				
					switch(debug_rx_buf[DEBUG_IDX_CMD])
					{
						case DEBUG_CMD_START_RTC:
							MX_RTC_Init();
							if(xSemaphoreTake(xSemaphore_send_log, 10000))
							{
								printf("RTC Initialized!\r\n");
								xSemaphoreGive(xSemaphore_send_log);
								break;
							}
							break;
							
						case DEBUG_CMD_SET_RTC:
							if(xSemaphoreTake(xSemaphore_send_log, 10000))
							{
								printf("Set RTC command received\r\n");
								xSemaphoreGive(xSemaphore_send_log);
							}

							date.Date = debug_rx_buf[DEBUG_IDX_DATA] ;
							date.Month = debug_rx_buf[DEBUG_IDX_DATA + 1];
							date.Year = debug_rx_buf[DEBUG_IDX_DATA + 2];
							time.Hours = debug_rx_buf[DEBUG_IDX_DATA + 3];
							time.Minutes = debug_rx_buf[DEBUG_IDX_DATA + 4];
							time.Seconds = debug_rx_buf[DEBUG_IDX_DATA + 5];
							time.TimeFormat = RTC_HOURFORMAT12_AM;
							time.SubSeconds = 0;
							time.SecondFraction = 0;
							
							if (HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK)
							{
								if(xSemaphoreTake(xSemaphore_send_log, 10000))
								{
									printf("Failed...\r\n");
									xSemaphoreGive(xSemaphore_send_log);
									break;
								}
							}
							if (HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
							{
								if(xSemaphoreTake(xSemaphore_send_log, 10000))
								{
									printf("Failed...\r\n");
									xSemaphoreGive(xSemaphore_send_log);
									break;
								}
							}
							//set alarm on on the next minute
							if(DEBUG_MODE)
							{
							   set_RTC_alarm_DEBUG();
							}
							else
							{
								set_RTC_alarm (SYS_RTC_ALARM_MIN);
							}
							print_timestamp();
							
							if(xSemaphoreTake(xSemaphore_send_log, 10000))
							{							
								printf("RTC set time successful! Alarm set every %d minutes\r\n", SYS_RTC_ALARM_MIN);
								xSemaphoreGive(xSemaphore_send_log);
								break;
							}
							break;
													
							//case DEBUG_CMD_SET_ALARM:
							//break;
							
						case DEBUG_CMD_GET_TIME:
							if(xSemaphoreTake(xSemaphore_send_log, 10000))
							{
								printf("Get Time command received\r\n");
								xSemaphoreGive(xSemaphore_send_log);
							}						
							print_timestamp();
							break;
							
						default:
							if(xSemaphoreTake(xSemaphore_send_log, 10000))
							{
								printf("Invalid command header = %d", debug_rx_buf[DEBUG_IDX_CMD]);
								xSemaphoreGive(xSemaphore_send_log);
								break;
							}
							break;
					}
					memset(debug_rx_buf, 0, sizeof(debug_rx_buf));
			}
		}
}

void set_RTC_alarm (uint8_t alrm_next_xminutes)
{
		uint8_t buf;
		uint32_t value;
		RTC_TimeTypeDef time;
		RTC_DateTypeDef date;
		RTC_AlarmTypeDef next_alarm;
	
		HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	
		time.Minutes += alrm_next_xminutes;

		correct_rtc_values(&time.Hours, &time.Minutes, &time.Seconds, &date.Month, &date.Date, &date.Year, RTC_FORMAT_BIN);
		
		next_alarm.AlarmTime.Hours = time.Hours;
		next_alarm.AlarmTime.Minutes = time.Minutes;
		next_alarm.AlarmTime.Seconds = time.Seconds;
		next_alarm.AlarmTime.SubSeconds = 0x0;
		next_alarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		next_alarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
		next_alarm.AlarmMask = RTC_ALARMMASK_NONE;
		next_alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
		next_alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
		next_alarm.AlarmDateWeekDay = date.Date;
		next_alarm.Alarm = RTC_ALARM_A;
		
		if (HAL_RTC_SetAlarm_IT(&hrtc, &next_alarm, RTC_FORMAT_BIN) != HAL_OK)
		{
			if(xSemaphoreTake(xSemaphore_send_log, 10000))
			{
				printf("Setting next alarm failed.");
				xSemaphoreGive(xSemaphore_send_log);
			}
		}
}

void set_RTC_alarm_DEBUG (void)
{
		uint8_t buf;
		uint32_t value;
		RTC_TimeTypeDef time;
		RTC_DateTypeDef date;
		RTC_AlarmTypeDef next_alarm;
	
		HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	
		time.Seconds += 3;

		correct_rtc_values(&time.Hours, &time.Minutes, &time.Seconds, &date.Month, &date.Date, &date.Year, RTC_FORMAT_BIN);
		
		next_alarm.AlarmTime.Hours = time.Hours;
		next_alarm.AlarmTime.Minutes = time.Minutes;
		next_alarm.AlarmTime.Seconds = time.Seconds;
		next_alarm.AlarmTime.SubSeconds = 0x0;
		next_alarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		next_alarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
		next_alarm.AlarmMask = RTC_ALARMMASK_NONE;
		next_alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
		next_alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
		next_alarm.AlarmDateWeekDay = date.Date;
		next_alarm.Alarm = RTC_ALARM_A;
		
		if (HAL_RTC_SetAlarm_IT(&hrtc, &next_alarm, RTC_FORMAT_BIN) != HAL_OK)
		{
			if(xSemaphoreTake(xSemaphore_send_log, 10000))
			{
				printf("Setting next alarm failed.");
				xSemaphoreGive(xSemaphore_send_log);
			}
		}
}

void correct_rtc_values(uint8_t* hour, uint8_t* minute, uint8_t* second, uint8_t* month, uint8_t* date, uint8_t* year, uint32_t time_format)
{ 
		if(time_format == RTC_FORMAT_BCD) {
//				if(*second > 0x59) 
//				{
//						*second -= 0x60;
//						*minute += 1;
						if(*minute > 0x59) 
						{
								*minute -= 0x60;
								*hour += 1;
								if(*hour > 0x23) 
								{
										*hour -= 0x24;
										*date += 1;
										
										//filter if alarm month has 30 or 31 days
										//months with 31 days
										if( (*date == 0x01) || (*date == 0x03) || (*date == 0x05) || (*date == 0x07) || 
											  (*date == 0x08) || (*date == 0x10) || (*date == 0x12))
										{
												if(*date > 0x31) 
												{
														*date -= 0x31;
														*month += 1;
														if(*month > 0x12)
														{
															*month -= 0x12;
															*year += 1;
														}															
												}
										}
										//months with 30 days
										else
										{
												if(*date > 0x30) 
												{
														*date -= 0x30;
														*month += 1;
														if(*month > 0x12)
														{
															*month -= 0x12;
															*year += 1;
														}															
												}											
										}
								}
						}
//				}
		}
		
		if(time_format == RTC_FORMAT_BIN) {
//				if(*second > 59) {
//						*second -= 60;
//						*minute += 1;
						if(*minute > 59) {
								*minute -= 60;
								*hour += 1;
								if(*hour > 23) {
										*hour -= 24;
										*date += 1;
									
									  //filter if alarm month has 30 or 31 days
										//months with 31 days
										if( (*date == 1) || (*date == 3) || (*date == 5) || (*date == 7) || 
											  (*date == 8) || (*date == 10) || (*date == 12))
										{
												if(*date > 31) 
												{
														*date -= 31;
														*month += 1;
														if(*month > 12)
														{
															*month -= 12;
															*year += 1;
														}															
												}
										}
										//months with 30 days
										else
										{
												if(*date > 30) 
												{
														*date -= 30;
														*month += 1;
														if(*month > 12)
														{
															*month -= 12;
															*year += 1;
														}															
												}											
										}									
								}
						}
				}			
//		}
}


void status_led_task ( void * pvParameters )
{
	while(1)
	{
		switch(sys_stat)
		{
			case SYS_STAT_IDLE:
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
				vTaskDelay(100 / portTICK_PERIOD_MS );
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
				vTaskDelay(3000 / portTICK_PERIOD_MS );
				break;

			default:
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				vTaskDelay(200 / portTICK_PERIOD_MS );
				break;
		}
	}
}

void convert_to_ascii(uint32_t number, uint8_t* output)//, uint8_t* hun, uint8_t* ten, uint8_t* ones)
{
	uint8_t hun, ten, ones = 0;
	
	hun = number / 100;	
	ones = number % 10;
	number = number - ones;
	ten = (number % 100) / 10;
	
	hun += 0x30;
	ten += 0x30;
	ones += 0x30;
	
	*output++ = hun;
  *output++ = ten;
	*output = ones;
}

void timer_uS_start(uint32_t period) 
{
  htim2.Init.Period = period;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    //Error_Handler();
  }	
	//HAL_TIM_Base_Start(&htim2); //orig code
	HAL_TIM_Base_Start_IT(&htim2);
	CLEAR_BIT(htim2.Instance->SR, TIM_SR_UIF);
}

void timer_uS_stop(void) 
{
	//HAL_TIM_Base_Stop(&htim2); //orig code
	HAL_TIM_Base_Stop_IT(&htim2);
}

void timer_delay_uS(uint32_t uS)
{
  timer_uS_start(uS * period_1uS);	
	while(!(READ_BIT(htim2.Instance->SR, TIM_SR_UIF)));
	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
	timer_uS_stop();	
}
