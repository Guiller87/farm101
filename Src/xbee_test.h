
#include "stm32l4xx_hal.h"

#define TX_SUCCESS 0
#define MAX_RETRY 5
#define MAX_WAIT_FOR_RESP 200


//Generic frame index definitions
#define START 0
#define LENGHT 1
#define FRAME_TYPE 3

//TXx Packet status frame index definitions
#define FRAME_ID 4
#define DEST_ADDR_LONG 5
#define DEST_ADDR 13
#define BROD_RAD 15
#define OPTIONS 16
#define TX_DATA 17

//Transmit status frame index definitions
#define DELIVER_STAT 8

//Rx Packet status frame index definitions
#define SRC_ADDR_64 4
#define SRC_ADDR_16 12
#define RF_DATA 15
#define MODEM_STATUS_VAL 4

//Frame type definitions
#define TX_REQ 0x10
#define TX_STAT 0x8B
#define RX_PACKET 0x90
#define MODEM_STATUS 0x8A


//Flag definitions
#define TX_OK 0x01

void xbee_tx(UART_HandleTypeDef * huart, uint8_t* dest_addr, char* text, uint8_t text_len, uint8_t* value, uint8_t value_len);
void xbee_rx_task(void * pvParameters);
void xbee_tx_packet_create(uint8_t* dest_addr);
void xbee_tx_packet_update(char* data, uint8_t len);
void xbee_tx_packet_send(UART_HandleTypeDef * huart);
void xbee_send_timestamp(void);
