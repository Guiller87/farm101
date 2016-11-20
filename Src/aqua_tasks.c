
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "aqua_tasks.h"
#include "printf_mod.h"
#include "main.h"
#include "am2302.h"
#include "task.h"
#include "xbee_test.h"


uint8_t msg_buf[] = "second task test";

RTC_AlarmTypeDef next_alarm;


extern uint8_t aqua_cmd_idx;
extern uint8_t aqua_status;
extern uint32_t circulation_count;
extern uint32_t temp1, temp2;

extern SemaphoreHandle_t xSemaphore_printf;

extern QueueHandle_t xQueue_parse_data;
extern QueueHandle_t xQueue_set_next_RTC_alarm;
extern QueueHandle_t xQueue_read_am2302;
extern QueueHandle_t xQueue_parse_xbee_rx;

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef time;
extern RTC_DateTypeDef date;
extern ADC_HandleTypeDef hadc2;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart5;

extern void MX_RTC_Init(void);
void Error_Handler(void);


//void read_am2302(void * pvParameters) {
//	
//		uint8_t buf;
//		uint32_t value;
//		uint8_t i = 0;
//			
//		uint8_t ret_val = 1;
//		uint32_t data = 0;
//		uint16_t humidity = 0;
//		uint16_t temperature = 0;
//	
//		//uint8_t xbee_tx_buf[30] = {0x7E, 0x00, 0x1A, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x65, 0x64, 0x65, 0x64, 0x74};
//		uint8_t xbee_tx_buf[95] = {0x7E, 0x00, 0x5B, 0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x40, 0xE7, 0xCB, 0x9A, 0xFF, 0xFE, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 
//															 0x67, 0x65, 0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x45, 0x44, 0x2E, 0x20, 0x66, 0x61, 0x72, 0x6D, 0x69, 0x6E, 0x67, 0x20, 0x31, 0x30, 0x31, 0x20, 0x69, 0x73, 0x20, 0x69, 0x6E, 0x20, 0x64, 0x65,
//															 0x76, 0x65, 0x6C, 0x70, 0x6D, 0x65, 0x6E, 0x74, 0x2E, 0x20, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x78, 0x62, 0x65, 0x65, 0x20, 0x6D, 0x6F, 0x64, 0x75, 0x6C, 0x65, 0x21, 0x9C};
//			
//		while(1){
//			if(xQueueReceive(xQueue_read_am2302, &buf, 1000)) {
//		
//					vTaskSuspendAll();
//					//taskENTER_CRITICAL();  //use vTaskSuspendAll() instead of taskENTER_CRITICAL(). xbee uart5 received can not receive properly if taskENTER_CRITICAL() is used.
//					ret_val = read_data_am2302(&data);
//					//taskEXIT_CRITICAL();
//					xTaskResumeAll();

//					print_timestamp();
//				
//					humidity = (uint16_t) ((data & 0xFFFF0000) >> 16);
//					temperature = (uint16_t) (data & 0xFFFF);
//					if(xSemaphoreTake(xSemaphore_printf, 0))
//					{					
//						if(!ret_val) {
//								printf("Humidity = %d.%d %%RH    ", (humidity / 10), (humidity % 10) );
//								if ((temperature & 0x8000) != 0) {
//										printf("-");
//								}
//								printf("Temperature = %d.%d C \r\n", ((temperature & 0x7fff) / 10), (temperature & 0x7fff) % 10);
//						}
//						else {
//							printf("am2302 error %d \r\n", ret_val);
//						}
//						xSemaphoreGive(xSemaphore_printf);
//						
//						HAL_UART_Transmit(&huart5, xbee_tx_buf, 95, 1000);
//					}				
//			}
//		}
//}






