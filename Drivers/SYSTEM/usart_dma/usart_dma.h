//#ifndef __USART_DMA_H
//#define __USART_DMA_H

//#include "stm32f4xx.h"
//#include <stdio.h>


//// === 内部配置宏 ===
//#define UART_INSTANCE           USART1
//#define UART_TX_PIN             GPIO_PIN_9
//#define UART_RX_PIN             GPIO_PIN_10
//#define UART_GPIO_PORT          GPIOA
//#define UART_GPIO_AF            GPIO_AF7_USART1

//#define RX_BUFFER_SIZE          256  // DMA接收缓冲区大小
//#define TX_RING_BUFFER_SIZE     512  // 发送环形缓冲区大小
//#define RX_RING_BUFFER_SIZE     512  // 接收环形缓冲区大小

//// === 模块内部静态变量，实现完全封装 ===
//static UART_HandleTypeDef s_uart_handle;
//static DMA_HandleTypeDef  s_dma_rx_handle;

//// DMA使用的线性接收缓冲区
//static uint8_t s_dma_rx_buffer[RX_BUFFER_SIZE];

//// 发送环形缓冲区
//static uint8_t  s_tx_ring_buffer[TX_RING_BUFFER_SIZE];
//static volatile uint16_t s_tx_ring_head = 0;
//static volatile uint16_t s_tx_ring_tail = 0;

//// 接收环形缓冲区
//static uint8_t  s_rx_ring_buffer[RX_RING_BUFFER_SIZE];
//static volatile uint16_t s_rx_ring_head = 0;
//static volatile uint16_t s_rx_ring_tail = 0;


//void UART_Init(uint32_t baudrate);

///**
// * @brief   发送指定长度的数据
// * @param   p_data  要发送的数据指针
// * @param   len     要发送的数据长度
// * @retval  实际放入发送缓冲区的数据长度
// */
//uint16_t UART_Transmit(const uint8_t *p_data, uint16_t len);

///**
// * @brief   从接收缓冲区读取指定长度的数据
// * @param   p_buffer    用于存放数据的缓冲区指针
// * @param   len         想要读取的数据长度
// * @retval  实际读取到的数据长度
// */
//uint16_t UART_Read(uint8_t *p_buffer, uint16_t len);

///**
// * @brief   获取接收缓冲区中当前有多少字节数据
// * @retval  可读取的数据字节数
// */
//uint16_t UART_GetRxBytesAvailable(void);


//#endif