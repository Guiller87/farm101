#include "stm32l4xx_hal.h"


#define OUTPUT 0
#define INPUT 1

/* DHT22_GetReadings response codes */
#define DHT22_RCV_OK               0 // Return with no error
#define DHT22_RCV_NO_RESPONSE      1 // No response from sensor
#define DHT22_RCV_BAD_ACK1         2 // Bad first half length of ACK impulse
#define DHT22_RCV_BAD_ACK2         3 // Bad second half length of ACK impulse
#define DHT22_RCV_RCV_TIMEOUT      4 // It was timeout while receiving bits
#define DHT22_RCV_TIMING_ERROR1     5 // 
#define DHT22_RCV_TIMING_ERROR2     6 // 
#define DHT22_RCV_CRC_ERROR				7

#define period_1uS 80

void Reinit_SW_Pin(GPIO_TypeDef* SWP_PORT, uint16_t SWP_Pin, uint8_t IO);
uint8_t read_data_am2302(GPIO_TypeDef* SWP_PORT, uint16_t SWP_Pin, uint32_t* data);

void timer_uS_start(uint32_t period);
void timer_uS_stop(void);
void timer_delay_uS(uint32_t uS);

