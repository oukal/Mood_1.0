///**
// ****************************************************************************************************
// * @file        usart.c
// * @author      ouka
// * @version     V1.0
// * @date        2025.11.12
// * @brief       串口初始化代码(一般是串口1)，支持printf

// * 修改说明
// * V1.0 2025 11.20
// * 删除USART_UX_IRQHandler()函数的超时处理和修改HAL_UART_RxCpltCallback()
// ****************************************************************************************************
// */


//#include "SYSTEM/sys/sys.h"
//#include "SYSTEM/usart_dma/usart_dma.h"

//#include <string.h>



//// === printf重定向 ===
//#pragma import(__use_no_semihosting)
//struct __FILE { int handle; };
//FILE __stdout;
//void _sys_exit(int x) { x = x; }
//int fputc(int ch, FILE *f)
//{
//    // 将printf的输出变成非阻塞的
//    UART_Transmit((uint8_t *)&ch, 1);
//    return ch;
//}

//// === 核心实现 ===

//void UART_Init(uint32_t baudrate)
//{
//    // ... GPIO和UART时钟使能 ...
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    __HAL_RCC_USART1_CLK_ENABLE();
//    __HAL_RCC_DMA2_CLK_ENABLE();

//    // ... GPIO初始化 ... (与原版类似)

//    // ... UART初始化 ... (与原版类似)
//    s_uart_handle.Instance = UART_INSTANCE;
//    // ...
//    HAL_UART_Init(&s_uart_handle);

//    // ... DMA RX 初始化 ...
//    s_dma_rx_handle.Instance = DMA2_Stream2;
//    // ... (配置DMA通道、方向、模式等) ...
//    HAL_DMA_Init(&s_dma_rx_handle);
//    __HAL_LINKDMA(&s_uart_handle, hdmarx, s_dma_rx_handle);

//    // === 关键：使能空闲中断(IDLE)和启动DMA接收 ===
//    __HAL_UART_ENABLE_IT(&s_uart_handle, UART_IT_IDLE);
//    HAL_UART_Receive_DMA(&s_uart_handle, s_dma_rx_buffer, RX_BUFFER_SIZE);

//    // ... NVIC中断配置 ...
//    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
//    HAL_NVIC_EnableIRQ(USART1_IRQn);
//    // ... (DMA中断也可以配置，用于错误处理) ...
//}

//uint16_t UART_Transmit(const uint8_t *p_data, uint16_t len)
//{
//    uint16_t i = 0;
//    for (i = 0; i < len; i++)
//    {
//        uint16_t next_head = (s_tx_ring_head + 1) % TX_RING_BUFFER_SIZE;
//        if (next_head == s_tx_ring_tail)
//        {
//            // 缓冲区已满，返回实际放入的长度
//            break;
//        }
//        s_tx_ring_buffer[s_tx_ring_head] = p_data[i];
//        s_tx_ring_head = next_head;
//    }

//    // 如果UART不在发送状态，则手动启动发送
//    if (__HAL_UART_GET_FLAG(&s_uart_handle, UART_FLAG_TXE) != RESET)
//    {
//        if (s_tx_ring_head != s_tx_ring_tail)
//        {
//            uint8_t data_to_send = s_tx_ring_buffer[s_tx_ring_tail];
//            s_tx_ring_tail = (s_tx_ring_tail + 1) % TX_RING_BUFFER_SIZE;
//            s_uart_handle.Instance->DR = data_to_send; // 写入数据
//            __HAL_UART_ENABLE_IT(&s_uart_handle, UART_IT_TXE); // 使能发送空中断
//        }
//    }
//    return i;
//}

//// ... UART_Read 和 UART_GetRxBytesAvailable 函数的实现 ...
//// (它们负责从 s_rx_ring_buffer 中安全地读取数据)

//// === 中断服务函数 ===
//void USART1_IRQHandler(void)
//{
//    // --- 空闲中断处理 ---
//    if (__HAL_UART_GET_FLAG(&s_uart_handle, UART_FLAG_IDLE) != RESET)
//    {
//        __HAL_UART_CLEAR_IDLEFLAG(&s_uart_handle);
//        HAL_UART_DMAStop(&s_uart_handle);

//        // 计算本次DMA接收到的数据长度
//        uint16_t rx_len = RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&s_dma_rx_handle);

//        // 将DMA缓冲区的数据，拷贝到接收环形缓冲区中
//        for (uint16_t i = 0; i < rx_len; i++)
//        {
//            uint16_t next_head = (s_rx_ring_head + 1) % RX_RING_BUFFER_SIZE;
//            if (next_head != s_rx_ring_tail)
//            {
//                s_rx_ring_buffer[s_rx_ring_head] = s_dma_rx_buffer[i];
//                s_rx_ring_head = next_head;
//            }
//            else { /* 环形缓冲区满，丢弃数据 */ }
//        }

//        // 重启DMA接收
//        HAL_UART_Receive_DMA(&s_uart_handle, s_dma_rx_buffer, RX_BUFFER_SIZE);
//    }

//    // --- 发送中断处理 ---
//    if (__HAL_UART_GET_FLAG(&s_uart_handle, UART_FLAG_TXE) != RESET)
//    {
//        if (s_tx_ring_head != s_tx_ring_tail)
//        {
//            uint8_t data_to_send = s_tx_ring_buffer[s_tx_ring_tail];
//            s_tx_ring_tail = (s_tx_ring_tail + 1) % TX_RING_BUFFER_SIZE;
//            s_uart_handle.Instance->DR = data_to_send;
//        }
//        else
//        {
//            // 发送环形缓冲区已空，关闭发送中断
//            __HAL_UART_DISABLE_IT(&s_uart_handle, UART_IT_TXE);
//        }
//    }
//}


