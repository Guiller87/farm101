
#include "stm32l4xx_hal.h"
#include "ultrasonic.h"
#include "main.h"
#include "util_func.h" //has the timer functions

extern TIM_HandleTypeDef htim2;

uint32_t measure_distance(GPIO_TypeDef* trig_port, uint32_t trig_pin, GPIO_TypeDef* echo_port, uint32_t echo_pin)
{
	uint32_t uS_count = 0;
	
	//trigger sensor
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
  timer_delay_uS(20);	
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
  timer_delay_uS(5);
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
	

  timer_uS_start(800 * period_1uS);	
	while(1)
	{
		if(READ_BIT(htim2.Instance->SR, TIM_SR_UIF))
		{
			__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
			timer_uS_stop();
			return DISTANCE_ERROR;
		}
		if(HAL_GPIO_ReadPin(echo_port, echo_pin) == 1)
		{
			break;
		}
	}


	while(HAL_GPIO_ReadPin(echo_port, echo_pin) == 1) 
	{
		timer_delay_uS(1);
		uS_count++;

		if(uS_count >= HCSR04_TIMEOUT)
		{
			return DISTANCE_ERROR; 
		}
	}
	timer_uS_stop();
	
	//divide uS_count by a constant to get the distance in cm
  return (uS_count / HCSR04_CONSTANT);
}