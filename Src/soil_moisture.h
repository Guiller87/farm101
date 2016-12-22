
#include "stm32l4xx_hal.h"


uint32_t read_soil_moisture(ADC_HandleTypeDef* hadc, uint32_t ADC_channel, uint8_t samples);