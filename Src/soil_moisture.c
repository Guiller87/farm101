
#include "stm32l4xx_hal.h"
#include "main.h"
#include "soil_moisture.h"

uint32_t read_soil_moisture(ADC_HandleTypeDef* hadc, uint32_t ADC_channel, uint8_t samples)
{
	  uint32_t value = 0;
	  uint8_t i;
	
    ADC_ChannelConfTypeDef sConfig;
	
	  sConfig.Channel = ADC_channel;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
		sConfig.SingleDiff = ADC_SINGLE_ENDED;
		sConfig.OffsetNumber = ADC_OFFSET_NONE;
		sConfig.Offset = 0;
		HAL_ADC_ConfigChannel(hadc, &sConfig);
	
		HAL_ADC_Start(hadc);
	
	  for(i = 0; i < samples; i++) {
				HAL_ADC_PollForConversion(hadc, 1000);
				value += HAL_ADC_GetValue(hadc);			
		}

		HAL_ADC_Stop(hadc);
	  return (value / samples);
}