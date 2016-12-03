#include "stm32l4xx_hal.h"


uint32_t measure_distance(GPIO_TypeDef* trig_port, uint32_t trig_pin, GPIO_TypeDef* echo_port, uint32_t echo_pin);
