

#include "stm32l4xx_hal.h"
#include "cmsis_os.h"


#define PUMP_ON 			HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_SET)
#define PUMP_OFF 			HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_RESET)
#define VALVE1_OPEN		HAL_GPIO_WritePin(VALVE1_GPIO_Port, VALVE1_Pin, GPIO_PIN_SET);
#define VALVE1_CLOSE	HAL_GPIO_WritePin(VALVE1_GPIO_Port, VALVE1_Pin, GPIO_PIN_RESET);
#define VALVE2_OPEN		HAL_GPIO_WritePin(VALVE2_GPIO_Port, VALVE2_Pin, GPIO_PIN_SET);
#define VALVE2_CLOSE	HAL_GPIO_WritePin(VALVE2_GPIO_Port, VALVE2_Pin, GPIO_PIN_RESET);

void read_am2302( void * pvParameters );
//void xbee_task( void * pvParameters );
