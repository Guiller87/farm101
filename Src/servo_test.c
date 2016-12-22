
#include "stm32l4xx_hal.h"
#include "main.h"
#include "servo_test.h"


void servo_set_degree(TIM_HandleTypeDef *htim, uint32_t Channel, float degree)
{
	TIM_OC_InitTypeDef sConfigOC;
	
	float unit_per_deg;
	
	unit_per_deg = (float)((float)(SERVO_180DEG_CONSTANT - SERVO_0DEG_CONSTANT) / (float)(180));
	
  sConfigOC.Pulse = (uint32_t)((float)(PEROID_20mS_CONSTANT) / ((float)(degree * unit_per_deg)));	
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	
	HAL_TIM_Base_Stop(htim);
	HAL_TIM_PWM_Stop(htim, Channel);
	
  HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, Channel);
	
	HAL_TIM_Base_Start(htim);
	HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3);
}

void servo_stop(TIM_HandleTypeDef *htim, uint32_t Channel)
{
	HAL_TIM_Base_Stop(htim);	
	HAL_TIM_PWM_Stop(htim, Channel);	
}