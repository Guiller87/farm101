
#include "stm32l4xx_hal.h"
#include "ultrasonic.h"
#include "main.h"
#include "am2302.h" //has the timer functions

extern TIM_HandleTypeDef htim2;

uint32_t measure_distance(GPIO_TypeDef* trig_port, uint32_t trig_pin, GPIO_TypeDef* echo_port, uint32_t echo_pin)
{
	uint32_t uS_count = 0;
	
	//trigger sensor
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
  timer_delay_uS(100);
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
	
	//start timer
	timer_delay_uS(SystemCoreClock / 1000000); //perid set to 1 uS
	while(HAL_GPIO_ReadPin(echo_port, echo_pin) == 0) { 
		if(READ_BIT(htim2.Instance->SR, TIM_SR_UIF))
		{
			uS_count++;
		}
		if(uS_count >= 100)
		{
			return 1; //error. range is set to upto 2 meters only.
		}
	}
	timer_uS_stop();
  return uS_count;
}