/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

//#include "stm32l4xx_hal.h"


/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define GARDEN_PUMP_Pin GPIO_PIN_2
#define GARDEN_PUMP_GPIO_Port GPIOC
#define AQUA_FISH_TANK_TURBIDITY_Pin GPIO_PIN_3
#define AQUA_FISH_TANK_TURBIDITY_GPIO_Port GPIOC
#define GARDEN_SOIL_MOISTURE_1_Pin GPIO_PIN_1
#define GARDEN_SOIL_MOISTURE_1_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define GARDEN_SOIL_MOISTURE_4_Pin GPIO_PIN_4
#define GARDEN_SOIL_MOISTURE_4_GPIO_Port GPIOA
#define GARDEN_SOIL_MOISTURE_3_Pin GPIO_PIN_7
#define GARDEN_SOIL_MOISTURE_3_GPIO_Port GPIOA
#define GARDEN_SOIL_MOISTURE_2_Pin GPIO_PIN_4
#define GARDEN_SOIL_MOISTURE_2_GPIO_Port GPIOC
#define AQUA_PUMP_TOWER_Pin GPIO_PIN_5
#define AQUA_PUMP_TOWER_GPIO_Port GPIOC
#define AQUA_PUMP_FISH_TANK_Pin GPIO_PIN_0
#define AQUA_PUMP_FISH_TANK_GPIO_Port GPIOB
#define GARDEN_TANK_LEVEL_TRIG_Pin GPIO_PIN_2
#define GARDEN_TANK_LEVEL_TRIG_GPIO_Port GPIOB
#define GARDEN_TANK_LEVEL_ECHO_Pin GPIO_PIN_10
#define GARDEN_TANK_LEVEL_ECHO_GPIO_Port GPIOB
#define STATUS_LED_Pin GPIO_PIN_12
#define STATUS_LED_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_13
#define LD2_GPIO_Port GPIOB
#define GARDEN_DHT22_Pin GPIO_PIN_15
#define GARDEN_DHT22_GPIO_Port GPIOB
#define GARDEN_PUMP_FLOW_METER_Pin GPIO_PIN_6
#define GARDEN_PUMP_FLOW_METER_GPIO_Port GPIOC
#define AQUA_PUMP_FISH_TANK_FLOW_METER_Pin GPIO_PIN_7
#define AQUA_PUMP_FISH_TANK_FLOW_METER_GPIO_Port GPIOC
#define AQUA_PUMP_TOWER_FLOW_METER_Pin GPIO_PIN_8
#define AQUA_PUMP_TOWER_FLOW_METER_GPIO_Port GPIOC
#define AQUA_FISH_TANK_LEVEL_ECHO_Pin GPIO_PIN_11
#define AQUA_FISH_TANK_LEVEL_ECHO_GPIO_Port GPIOA
#define AQUA_FISH_TANK_LEVEL_TRIG_Pin GPIO_PIN_12
#define AQUA_FISH_TANK_LEVEL_TRIG_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define AQUA_FISH_FOOD_LEVEL_ECHO_Pin GPIO_PIN_15
#define AQUA_FISH_FOOD_LEVEL_ECHO_GPIO_Port GPIOA
#define VALVE2_Pin GPIO_PIN_10
#define VALVE2_GPIO_Port GPIOC
#define XBEE_RESET_Pin GPIO_PIN_11
#define XBEE_RESET_GPIO_Port GPIOC
#define UART5_TX_XBEE_Pin GPIO_PIN_12
#define UART5_TX_XBEE_GPIO_Port GPIOC
#define UART5_RX_XBEE_Pin GPIO_PIN_2
#define UART5_RX_XBEE_GPIO_Port GPIOD
#define AQUA_FISH_FOOD_LEVEL_TRGI_Pin GPIO_PIN_3
#define AQUA_FISH_FOOD_LEVEL_TRGI_GPIO_Port GPIOB
#define SPI1_NSS_Pin GPIO_PIN_6
#define SPI1_NSS_GPIO_Port GPIOB
#define AQUA_FISH_FEEDER_Pin GPIO_PIN_8
#define AQUA_FISH_FEEDER_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */


#define SYS_RTC_ALARM_MIN 1

#define DEBUG_IDX_DELIMITER 0
#define DEBUG_IDX_CMD 1
#define DEBUG_IDX_LEN 2
#define DEBUG_IDX_DATA 3

#define SHOW_TIME_EVERY_NMSECONDS 1000
#define PUMP_ON_EVERY_NSECONDS 0x02
#define PUMP_ON_EVERY_NMINUTES 1


#define SYS_STAT_IDLE 0x00

//garden status
#define SYS_STAT_GARDEN_WATERING_MASK (0x01 << 0)
#define SYS_STAT_GARDEN_CHECKING_WATERFLOW (0x01 << 1)
#define SYS_STAT_GARDEN_CHECKING_HUMID_TEMP (0x01 << 2)
#define SYS_STAT_GARDEN_CHECKING_SOIL_MOISTURE (0x01 << 3)
#define SYS_STAT_GARDEN_CHECKING_WATER_TANK (0x01 << 4)
#define SYS_STAT_AQUA_WATERING_MASK (0x01 << 5)
#define SYS_STAT_AQUA_CHECKING_FISH_FOOD (0x01 << 6)
#define SYS_STAT_AQUA_CHECKING_FISH_TANK (0x01 << 7)
#define SYS_STAT_AQUA_FEEDING_FISH (0x01 << 8)

#define DEBUG_RX_BUF_SIZE 50
#define XBEE_RX_BUF_SIZE 100
#define XBEE_TX_BUF_SIZE 100

#define GARDEN_FLOW_SENSOR_TIM htim3
#define GARDEN_FLOW_SENSOR_CH TIM_CHANNEL_1


/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
