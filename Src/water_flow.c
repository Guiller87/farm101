
#include "stm32l4xx_hal.h"
#include "main.h"
#include "water_flow.h"

extern uint8_t GARDEN_PUMP_FLOW_METER_count;
extern uint8_t AQUA_PUMP_FISH_TANK_FLOW_METER_count;
extern uint8_t AQUA_PUMP_TOWER_FLOW_METER_count;

void start_flow_rate(uint16_t gpio_pin)
{
		switch (gpio_pin)
		{
			case GARDEN_PUMP_FLOW_METER_Pin:
					GARDEN_PUMP_FLOW_METER_count = 0;
					break;
			
			case AQUA_PUMP_FISH_TANK_FLOW_METER_Pin:
				  AQUA_PUMP_FISH_TANK_FLOW_METER_count = 0;
				  break;
			
			case AQUA_PUMP_TOWER_FLOW_METER_Pin:
				  AQUA_PUMP_TOWER_FLOW_METER_count = 0;
			    break;
		}
}	

uint32_t read_flow_rate(uint16_t gpio_pin)
{
		switch (gpio_pin)
		{
			case GARDEN_PUMP_FLOW_METER_Pin:
					return GARDEN_PUMP_FLOW_METER_count;

			
			case AQUA_PUMP_FISH_TANK_FLOW_METER_Pin:
				  return AQUA_PUMP_FISH_TANK_FLOW_METER_count;

			
			case AQUA_PUMP_TOWER_FLOW_METER_Pin:
				  return AQUA_PUMP_TOWER_FLOW_METER_count;

		}
}
