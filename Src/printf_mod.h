

#ifndef _PRINTF_USBFS_H
#define _PRINTF_USBFS_H

#include "stm32l4xx_hal.h"
#include "stdio.h"

extern UART_HandleTypeDef huart2;

struct __FILE {
	int dummy;
};

extern FILE __stdout;
extern int fputc(int ch, FILE *f);


//int fputc(int ch, FILE *f) {
//    /* Do your stuff here */
//    /* Send your custom byte */
//    /* Send byte to USART */
//		uint8_t buf[2] = {0};
//		uint32_t count;
//		buf[0] = ch;
//    HAL_Delay(1);	
//		
//    //CDC_Transmit_FS(buf,1);
//		HAL_UART_Transmit(&huart2, buf, 1, 1000);
//		
//    /* If everything is OK, you have to return character written */
//    return ch;
//    /* If character is not correct, you can return EOF (-1) to stop writing */
//    //return -1;
//}

#endif
