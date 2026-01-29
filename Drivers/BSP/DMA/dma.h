/**
 ****************************************************************************************************
 * @file        dma.h
 * @author      ouka
 * @version     V1.0
 * @date        2025 11.21
 * @brief       DMA 驱动代码
 * 修改说明
 * V1.0 2025 11.21
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __DMA_H
#define	__DMA_H

#include "./SYSTEM/sys/sys.h"

 
// 缓冲区大小定义 (根据实际需求调整)
// DMA接收缓冲 (线性缓冲，用于DMA搬运的一级缓存)
#define USART_DMA_RX_BUF_SIZE   256 
// 软件接收环形缓冲 (二级缓存，主循环从这里读)
#define USART_RX_RING_SIZE      1024 
// 软件发送环形缓冲 (发送队列)
#define USART_TX_RING_SIZE      1024 



extern DMA_HandleTypeDef g_dma_handle; 

// --- 函数声明 ---
//初始化
void dma_init(DMA_Stream_TypeDef *dma_stream_handle, uint32_t ch);  /* 配置DMAx_CHx */
void USART_WriteData(const uint8_t *data, uint16_t len);
uint16_t USART_ReadData(uint8_t *buf, uint16_t len);     // 核心读取函数
uint16_t USART_GetRxAvailable(void);           // 查询有多少数据可读
void USART_UX_IRQHandler_DMA(void);           //中断处理函数
#endif


