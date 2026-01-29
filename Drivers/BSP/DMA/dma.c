 /**
 ****************************************************************************************************
 * @file        dma.c
 * @author      ouka
 * @version     V1.0
 * @date        2025-11-21
 * @brief       DMA 驱动代码
                串口1 初始化 + DMA2接收不定长数据 + 环形队列发送
 * 修改说明
 * V1.0 
 ****************************************************************************************************
 */

#include "./BSP/DMA/dma.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/sys/sys.h"
#include <string.h>
//变量定义

DMA_HandleTypeDef  g_dma_handle;                  /* DMA句柄 */
// 1. DMA 接收相关
// DMA 直接搬运的“线性”缓冲区
static uint8_t g_dma_rx_buf[USART_DMA_RX_BUF_SIZE]; 
static uint16_t g_dma_old_pos = 0; // 双指针算法的核心：记录上一次读取到的位置

// 2. 接收环形缓冲区 (Application Ring Buffer)
static uint8_t g_rx_ring_buf[USART_RX_RING_SIZE];
static volatile uint16_t g_rx_head = 0; // 写入位置 (ISR写)
static volatile uint16_t g_rx_tail = 0; // 读取位置 (APP读)

// 3. 发送环形缓冲区
static uint8_t g_tx_ring_buf[USART_TX_RING_SIZE];
static volatile uint16_t g_tx_head = 0; // 写入位置 (APP写)
static volatile uint16_t g_tx_tail = 0; // 读取位置 (ISR读)


/**
 * @brief       串口 DMA初始化函数
 *   @note      这里的传输形式是固定的, 这点要根据不同的情况来修改
 *              从存储器 -> 外设模式/8位数据宽度/存储器增量模式
 *
 * @param       dma_stream_handle : DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
 * @retval      无
 */
void dma_init(DMA_Stream_TypeDef *dma_stream_handle, uint32_t ch)
{ 
    if ((uint32_t)dma_stream_handle > (uint32_t)DMA2)       /* 得到当前stream是属于DMA2还是DMA1 */
    {
        __HAL_RCC_DMA2_CLK_ENABLE();                        /* DMA2时钟使能 */
    }
    else
    {
        __HAL_RCC_DMA1_CLK_ENABLE();                        /* DMA1时钟使能 */
    }

    __HAL_LINKDMA(&g_uart1_handle, hdmatx, g_dma_handle);   /* 将DMA与USART1联系起来(发送DMA) */

    /* Tx DMA配置 */
    g_dma_handle.Instance = dma_stream_handle;                    /* 数据流选择 */
    g_dma_handle.Init.Channel = ch;                               /* DMA通道选择 */
    g_dma_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;           /* 存储器到外设 */
    g_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;               /* 外设非增量模式 */
    g_dma_handle.Init.MemInc = DMA_MINC_ENABLE;                   /* 存储器增量模式 */
    g_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  /* 外设数据长度:8位 */
    g_dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     /* 存储器数据长度:8位 */
    g_dma_handle.Init.Mode = DMA_CIRCULAR;                          /* 外设流控模式 */
    g_dma_handle.Init.Priority = DMA_PRIORITY_MEDIUM;             /* 中等优先级 */
    g_dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;            /* 关闭FIFO模式 */
    g_dma_handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;    /* FIFO阈值配置 */
    g_dma_handle.Init.MemBurst = DMA_MBURST_SINGLE;               /* 存储器突发单次传输 */
    g_dma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;            /* 外设突发单次传输 */

    HAL_DMA_DeInit(&g_dma_handle);
    HAL_DMA_Init(&g_dma_handle);
    // 5. 核心配置：启动 IDLE 中断 和 DMA 接收
    // 开启串口空闲中断
    __HAL_UART_ENABLE_IT(&g_uart1_handle, UART_IT_IDLE);
    // 启动DMA接收，数据存入线性缓冲 g_dma_rx_buf
    HAL_UART_Receive_DMA(&g_uart1_handle, g_dma_rx_buf, USART_DMA_RX_BUF_SIZE);

    // 6. 配置 NVIC
    HAL_NVIC_SetPriority(USART1_IRQn, 3, 3);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    
    // 注意：通常也需要开启 DMA 中断以处理传输完成或错误，但在 IDLE 模式下非必须

}




/**
 * @brief  非阻塞发送函数 (写入环形缓冲区，由中断负责发送)
 * @param  data: 数据指针
 * @param  len:  数据长度
 */
void USART_WriteData(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint16_t next_head;

    // 1. 将数据压入软件环形缓冲区
    for (i = 0; i < len; i++)
    {
        next_head = (g_tx_head + 1) % USART_TX_RING_SIZE;
        if (next_head == g_tx_tail)
        {
            // 缓冲区已满，丢弃剩余数据或在此处阻塞等待
            break; 
        }
        g_tx_ring_buf[g_tx_head] = data[i];
        g_tx_head = next_head;
    }

    // 2. 触发发送
    // 如果 TXE 中断没开，说明之前发送完了或者没开始，需要手动开启
    // 开启后，硬件检测到 DR 寄存器为空，会立即进入 ISR 搬运第一个字节
    if (__HAL_UART_GET_IT_SOURCE(&g_uart1_handle, UART_IT_TXE) == RESET)
    {
        // 只有当环形缓冲区有数据时才开启
        if (g_tx_head != g_tx_tail)
        {
            __HAL_UART_ENABLE_IT(&g_uart1_handle, UART_IT_TXE);
        }
    }
}

/**
 * @brief  查询接收缓冲区有多少有效数据
 */
uint16_t USART_GetRxAvailable(void)
{
    if (g_rx_head >= g_rx_tail)
    {
        return g_rx_head - g_rx_tail;
    }
    else
    {
        return USART_RX_RING_SIZE - g_rx_tail + g_rx_head;
    }
}

/**
 * @brief  从接收环形缓冲区读取数据 (应用层调用)
 * @param  buf: 用户缓冲区
 * @param  len: 想读取的长度
 * @return 实际读取的长度
 */
uint16_t USART_ReadData(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    uint16_t available = USART_GetRxAvailable();

    if (available < len)
    {
        len = available; // 数据不够，有多少读多少
    }

    for (i = 0; i < len; i++)
    {
        buf[i] = g_rx_ring_buf[g_rx_tail];
        g_rx_tail = (g_rx_tail + 1) % USART_RX_RING_SIZE;
    }

    return len;
}

/**
 * @brief  串口1 中断服务函数
 *         处理 IDLE (接收结束) 和 TXE (发送寄存器空)
 */


void USART_UX_IRQHandler_DMA(void)
{
    uint32_t flag = g_uart1_handle.Instance->SR;
    uint32_t cr1  = g_uart1_handle.Instance->CR1;

    // ================== 接收逻辑 (DMA Circular + IDLE) ==================
    if ((flag & UART_FLAG_IDLE) != RESET)
    {
        // 1. 清除 IDLE 标志 (F4: 读SR -> 读DR)
        __HAL_UART_CLEAR_IDLEFLAG(&g_uart1_handle);

        // 2. 获取 DMA 当前写入位置 (NDTR 是倒计数，所以要用 总大小 - CNDTR)
        // 循环模式不用停止 DMA
        uint16_t pos_now = USART_DMA_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(&g_dma_handle);
        
        // 3. 计算本次新收到的数据长度 (处理 DMA 环形卷绕)
        if (pos_now != g_dma_old_pos)
        {
            // 这里不需要申请大数组，直接一个字节一个字节搬，或者分两段 memcpy
            // 为了逻辑简单，这里演示逐字节搬运
            
            uint16_t process_len;
            if (pos_now > g_dma_old_pos) 
				process_len = pos_now - g_dma_old_pos;
            else 
				process_len = USART_DMA_RX_BUF_SIZE - g_dma_old_pos + pos_now;

            // 开始搬运
            for (uint16_t i = 0; i < process_len; i++)
            {
                // 计算在线性缓冲中的真实索引
                uint16_t real_idx = (g_dma_old_pos + i) % USART_DMA_RX_BUF_SIZE;
                
                // 放入软件环形缓冲
                uint16_t next_head = (g_rx_head + 1) % USART_RX_RING_SIZE;
                if (next_head != g_rx_tail)
                {
                    g_rx_ring_buf[g_rx_head] = g_dma_rx_buf[real_idx];
                    g_rx_head = next_head;
                }
            }
            // 更新旧指针
            g_dma_old_pos = pos_now;
        }
    }

    // ================== 发送逻辑 (TXE 中断) ==================
    if (((flag & UART_FLAG_TXE) != RESET) && ((cr1 & USART_CR1_TXEIE) != RESET))
    {
        if (g_tx_head != g_tx_tail)
        {
            // 缓冲区有数据，塞入 DR，硬件会自动清除 TXE
            g_uart1_handle.Instance->DR = g_tx_ring_buf[g_tx_tail];
            g_tx_tail = (g_tx_tail + 1) % USART_TX_RING_SIZE;
        }
        else
        {
            // 缓冲区空了，关闭 TXE 中断
            __HAL_UART_DISABLE_IT(&g_uart1_handle, UART_IT_TXE);
        }
    }
}

