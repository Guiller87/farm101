

#define TX_SUCCESS 0

//Generic frame index denifinitions
#define START 0
#define LENGHT 1
#define FRAME_TYPE 3


//Transmit status frame index denifinitions
#define DELIVER_STAT 8

//Rx Packet status frame index denifinitions
#define SRC_ADDR_64 4
#define SRC_ADDR_16 12
#define RF_DATA 15

//Frame type denifinitions
#define TX_STAT 0x8B
#define RX_PACKET 0x90


void xbee_rx_task(void * pvParameters);
