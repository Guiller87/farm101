
#include "stm32l4xx_hal.h"


#define OFF 0
#define ON 1

void pump_switch (GPIO_TypeDef* pump_port, uint16_t pump_pin, uint8_t state);