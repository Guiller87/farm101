#include "stm32l4xx_hal.h"



#define DEBUG_CMD_START_RTC 0x10
#define DEBUG_CMD_SET_RTC 0x11
#define DEBUG_CMD_SET_ALARM 0x12
#define DEBUG_CMD_GET_TIME 0x13

#define period_1uS 80


void status_led_task(void * pvParameters);
void debug_parse_data_rx(void * pvParameters);
void set_RTC_alarm (uint8_t alrm_next_xminutes);
void correct_rtc_values(uint8_t* hour, uint8_t* minute, uint8_t* second, \
												uint8_t* month, uint8_t* date, uint8_t* year, uint32_t time_format);
void print_timestamp(void);
void convert_to_ascii(uint32_t number, uint8_t* output);

void timer_uS_start(uint32_t period);
void timer_uS_stop(void);
void timer_delay_uS(uint32_t uS);

void set_RTC_alarm_DEBUG(void);