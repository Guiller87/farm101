
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "am2302.h"
#include "main.h"

extern TIM_HandleTypeDef htim2;

GPIO_InitTypeDef GPIO_InitStruct;
extern uint32_t counter;

uint8_t read_data_am2302(GPIO_TypeDef* swp_port, uint16_t swp_pin, uint32_t* data)
{
	uint8_t i, temp_crc = 0;	
	uint32_t pulse_width = 0;
	uint32_t temp_data = 0;	
	
	Reinit_SW_Pin(swp_port, swp_pin, OUTPUT);
	HAL_GPIO_WritePin(swp_port, swp_pin, GPIO_PIN_SET);
  timer_delay_uS(3000);
	
	//send start signal. should be atleast 1ms
	HAL_GPIO_WritePin(swp_port, swp_pin, GPIO_PIN_RESET);
  timer_delay_uS(2000);
	
	HAL_GPIO_WritePin(swp_port, swp_pin, GPIO_PIN_SET);
	
	//wait for am2302 respons for 20-40us
	Reinit_SW_Pin(swp_port, swp_pin, INPUT);

	timer_uS_start(50 * period_1uS);	
	while(HAL_GPIO_ReadPin(swp_port, swp_pin) == 1) {
		if(READ_BIT(htim2.Instance->SR, TIM_SR_UIF))
			return DHT22_RCV_NO_RESPONSE;
	}
	timer_uS_stop();
	
	//response signal 1 (low) should be around 80uS
	timer_uS_start(90 * period_1uS);	
	while(HAL_GPIO_ReadPin(swp_port, swp_pin) == 0) {
		if(READ_BIT(htim2.Instance->SR, TIM_SR_UIF))
			return DHT22_RCV_BAD_ACK1;
	}
	timer_uS_stop();	

	//response signal 2 (high) should be around 80uS	
	timer_uS_start(90 * period_1uS);	
	while(HAL_GPIO_ReadPin(swp_port, swp_pin) == 1) {
		if(READ_BIT(htim2.Instance->SR, TIM_SR_UIF))
			return DHT22_RCV_BAD_ACK2;
	}
	timer_uS_stop();

	
	// start reading the 40 bit temp and humidity data
	for(i = 0; i < 40; i++) {
		 
		//low signal is 50us. therefore longer than 50us of low is an error.	
		while(HAL_GPIO_ReadPin(swp_port, swp_pin) == 1); 
		timer_uS_start(70 * period_1uS);
		while(HAL_GPIO_ReadPin(swp_port, swp_pin) == 0) {
			if(READ_BIT(htim2.Instance->SR, TIM_SR_UIF))
				return DHT22_RCV_RCV_TIMEOUT;
		}
		timer_uS_stop();	
		
		//High bit is 70us. therefor longer than 70us of high is an error.		
		timer_uS_start(80 * period_1uS);	
		while(HAL_GPIO_ReadPin(swp_port, swp_pin) == 1) {
			if(READ_BIT(htim2.Instance->SR, TIM_SR_UIF))
				return DHT22_RCV_TIMING_ERROR1;
		}
		pulse_width = htim2.Instance->CNT;
		pulse_width = pulse_width / period_1uS;
		timer_uS_stop();
		
		
		if( (pulse_width >= 15) && (pulse_width <= 35) ) {
			temp_data = temp_data;
		}
		
		else {
			if( (pulse_width >= 60) && (pulse_width <= 80)) {
				if(i < 32) {
					temp_data |= 0x80000000 >> i;
				}
				else {
					temp_crc |= (0x80 >> (i - 32));
				}
			}
			else {
				return DHT22_RCV_TIMING_ERROR2;
			}
		}
	}

	if( (uint8_t)(temp_crc) == (uint8_t) (
														((uint8_t)(temp_data & 0x000000FF)) +
														(((uint8_t)((temp_data & 0x0000FF00) >> 8))) +	
														(((uint8_t)((temp_data & 0x00FF0000) >> 16))) +
														(((uint8_t)((temp_data & 0xFF000000) >> 24))) ) ) 
	{						
			*data = temp_data;
	}
	else {
		return DHT22_RCV_CRC_ERROR;
	}
	
	
	return 0;
}

void Reinit_SW_Pin(GPIO_TypeDef* swp_port, uint16_t swp_pin, uint8_t IO)
{	
	if(IO == OUTPUT){
			/*Configure GPIO pins : SINGLE_WIRE_Pin */
		GPIO_InitStruct.Pin = swp_pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(swp_port, &GPIO_InitStruct);
	}	
	
	if(IO == INPUT){
		GPIO_InitStruct.Pin = swp_pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(swp_port, &GPIO_InitStruct);
	}		
}

void timer_uS_start(uint32_t period) 
{
  htim2.Init.Period = period;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    //Error_Handler();
  }	
	HAL_TIM_Base_Start(&htim2);
	CLEAR_BIT(htim2.Instance->SR, TIM_SR_UIF);
}

void timer_uS_stop(void) 
{
		HAL_TIM_Base_Stop(&htim2);
}

void timer_delay_uS(uint32_t uS)
{
  timer_uS_start(uS * period_1uS);	
	while(!(READ_BIT(htim2.Instance->SR, TIM_SR_UIF)));
	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
	timer_uS_stop();	
}
