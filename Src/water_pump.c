
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "main.h"
#include "water_pump.h"


void pump_switch (GPIO_TypeDef* pump_port, uint16_t pump_pin, uint8_t state)
{
	if(state)
	{
		HAL_GPIO_WritePin(pump_port, pump_pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(pump_port, pump_pin, GPIO_PIN_RESET);	
	}
}