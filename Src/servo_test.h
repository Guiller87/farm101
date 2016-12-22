#include "stm32l4xx_hal.h"

#define PEROID_20mS_CONSTANT 0x9C40

#define SERVO_0DEG_CONSTANT 4
#define SERVO_180DEG_CONSTANT 32


void servo_set_degree(TIM_HandleTypeDef *htim, uint32_t Channel, float degree);
void servo_stop(TIM_HandleTypeDef *htim, uint32_t Channel);