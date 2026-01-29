# 🚀 FreeRTOS 移植完整指南（STM32F407）

## 📋 目录
1. [前置准备](#前置准备)
2. [FreeRTOS 源码获取](#freertos-源码获取)
3. [文件结构配置](#文件结构配置)
4. [配置文件设置](#配置文件设置)
5. [中断处理配置](#中断处理配置)
6. [主程序初始化](#主程序初始化)
7. [任务创建和管理](#任务创建和管理)
8. [现有代码集成](#现有代码集成)
9. [测试和调试](#测试和调试)

---

## 前置准备

### 系统信息
```
芯片: STM32F407VGT6
芯片主频: 168 MHz
RAM: 192 KB SRAM
ROM: 1 MB Flash

当前软件:
- STM32CubeMX (用于生成代码)
- Keil MDK-ARM (编译环境)
- HAL 库
```

### 需要下载的资源

1. **FreeRTOS 源码**
   - 官方网站: https://www.freertos.org/
   - 推荐版本: FreeRTOS V10.4.3+ (稳定版)
   - 下载: ZIP 格式

2. **STM32 相关文件**
   - STM32F4xx HAL 驱动（已有）
   - CMSIS 文件（已有）

---

## FreeRTOS 源码获取

### 方案 1: 从官网下载

```
1. 访问 https://www.freertos.org/download-freertos-plus.html
2. 选择最新的稳定版本（如 V10.4.3）
3. 下载完整的 ZIP 包
4. 解压到本地
```

### 方案 2: 使用 Git (推荐)

```bash
cd d:\desktop\Mood_1.0
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS
cd FreeRTOS
git checkout V10.4.3  # 检出稳定版本
```

### 关键文件说明

从 FreeRTOS 包中，你需要以下文件：

```
FreeRTOS-Kernel/
├── tasks.c                    ✅ 核心任务管理
├── queue.c                    ✅ 队列管理
├── timers.c                   ✅ 定时器
├── croutine.c                 ✅ 协程
├── event_groups.c             ✅ 事件组
├── stream_buffer.c            ✅ 流缓冲
├── include/
│   ├── FreeRTOS.h             ✅ 主头文件
│   ├── task.h                 ✅ 任务 API
│   ├── queue.h                ✅ 队列 API
│   ├── timers.h               ✅ 定时器 API
│   └── ... 其他头文件
├── portable/
│   ├── GCC/
│   │   └── ARM_CM4F/         ✅ ARM Cortex-M4F (STM32F407)
│   │       ├── port.c
│   │       └── portmacro.h
│   └── ... 其他编译器
└── MemMang/
    ├── heap_1.c               (1 级堆)
    ├── heap_2.c               (2 级堆，不推荐)
    ├── heap_3.c               (3 级堆，推荐)
    ├── heap_4.c               (4 级堆，最推荐)
    └── heap_5.c               (5 级堆，高级用途)
```

---

## 文件结构配置

### 第一步: 创建 FreeRTOS 文件夹结构

```
d:\desktop\Mood_1.0\
├── FreeRTOS/
│   ├── src/
│   │   ├── tasks.c
│   │   ├── queue.c
│   │   ├── timers.c
│   │   ├── croutine.c
│   │   ├── event_groups.c
│   │   ├── stream_buffer.c
│   │   └── heap_4.c (选择堆实现)
│   └── include/
│       ├── FreeRTOS.h
│       ├── task.h
│       ├── queue.h
│       ├── ... 其他头文件
├── Drivers/
├── Middlewares/
├── User/
│   ├── FreeRTOSConfig.h  (👈 最重要，需要自己创建)
│   └── main.c
└── Projects/
```

### 第二步: 复制文件

**复制核心源文件**：
```
FreeRTOS/ 源目录
└── tasks.c, queue.c, timers.c, croutine.c, event_groups.c, stream_buffer.c
    → 复制到 d:\desktop\Mood_1.0\FreeRTOS\src\

FreeRTOS/ 源目录
└── portable/GCC/ARM_CM4F/
    ├── port.c
    └── portmacro.h
    → 复制到 d:\desktop\Mood_1.0\FreeRTOS\portable\ARM_CM4F\

FreeRTOS/ 源目录
└── MemMang/heap_4.c
    → 复制到 d:\desktop\Mood_1.0\FreeRTOS\src\

FreeRTOS/ 源目录
└── include/
    ├── FreeRTOS.h
    ├── task.h
    ├── queue.h
    └── ... 所有头文件
    → 复制到 d:\desktop\Mood_1.0\FreeRTOS\include\
```

---

## 配置文件设置

### 创建 FreeRTOSConfig.h

这是最关键的配置文件。在 `d:\desktop\Mood_1.0\User\` 下创建 `FreeRTOSConfig.h`：

<details>
<summary>点击展开 FreeRTOSConfig.h 完整配置</summary>

```c
/*
 * FreeRTOS.org V10.4.3 Configuration for STM32F407
 * 这是 FreeRTOS 的核心配置文件
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ========================== 基础配置 ========================== */

/**
 * 应用程序专有定义。
 *
 * 这些定义应在构建系统中进行定义，而不是在这个文件中编辑
 */
#define configUSE_PREEMPTION                    1   /* 使用抢占式调度 */
#define configUSE_IDLE_HOOK                     0   /* 使用 Idle Hook */
#define configUSE_TICK_HOOK                     0   /* 使用 Tick Hook */
#define configCPU_CLOCK_HZ                      (168000000UL)  /* 168 MHz */
#define configTICK_RATE_HZ                      (1000UL)       /* 1ms 时钟周期 */
#define configMAX_PRIORITIES                    (5)             /* 最多 5 个优先级 */
#define configMINIMAL_STACK_SIZE                (128)           /* 最小栈大小（字） */
#define configTOTAL_HEAP_SIZE                   (32 * 1024)     /* 总堆大小（32KB） */
#define configMAX_TASK_NAME_LEN                 (16)            /* 任务名最大长度 */
#define configUSE_TRACE_FACILITY                1               /* 使用追踪功能 */
#define configUSE_16_BIT_TICKS                  0               /* 使用 32 位 Tick */

/**
 * 钩子函数配置
 */
#define configCHECK_FOR_STACK_OVERFLOW          2               /* 栈溢出检查（推荐值 2） */
#define configRECORD_STACK_HIGH_ADDRESS         1               /* 记录堆栈高地址 */

/**
 * 运行时间统计和任务监控
 */
#define configGENERATE_RUN_TIME_STATS           0               /* 不生成运行时间统计 */
#define configUSE_STATS_FORMATTING_FUNCTIONS    0               /* 不使用格式化函数 */

/* ========================== 队列和消息相关 ========================== */

#define configUSE_QUEUE_SETS                    0               /* 不使用队列集合 */
#define configUSE_MUTEXES                       1               /* 使用互斥锁 */
#define configUSE_RECURSIVE_MUTEXES             1               /* 使用递归互斥锁 */
#define configUSE_COUNTING_SEMAPHORES           1               /* 使用计数信号量 */
#define configUSE_ALTERNATIVE_API               0               /* 不使用旧 API */
#define configQUEUE_REGISTRY_SIZE               0               /* 队列注册表大小 */

/* ========================== 定时器相关 ========================== */

#define configUSE_TIMERS                        1               /* 启用软件定时器 */
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)  /* 定时器任务优先级 */
#define configTIMER_QUEUE_LENGTH                10              /* 定时器队列长度 */
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

/* ========================== 协程相关 ========================== */

#define configUSE_CO_ROUTINES                   0               /* 不使用协程 */
#define configMAX_CO_ROUTINE_PRIORITIES         (2)             /* 协程优先级数 */

/* ========================== 事件组相关 ========================== */

#define configUSE_EVENT_GROUPS                  1               /* 使用事件组 */

/* ========================== 流缓冲相关 ========================== */

#define configUSE_STREAM_BUFFERS                1               /* 使用流缓冲 */

/* ========================== 内存分配相关 ========================== */

#define configSUPPORT_DYNAMIC_ALLOCATION        1               /* 支持动态分配 */
#define configSUPPORT_STATIC_ALLOCATION         0               /* 不支持静态分配 */
#define configAPPLICATION_ALLOCATED_RAM         0               /* 应用申请内存 */

/* ========================== 任务删除相关 ========================== */

#define configUSE_TASK_NOTIFICATIONS            1               /* 使用任务通知 */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1               /* 通知数组大小 */

/* ========================== 清理函数 ========================== */

#define configUSE_DAEMON_TASK_DELETION          0               /* 使用后台任务删除 */

/* ========================== 系统调用相关 ========================== */

#define configENABLE_BACKWARD_COMPATIBILITY     0               /* 禁用向后兼容 */

/* ========================== 断言相关 ========================== */

#define configASSERT( x ) if((x) == 0) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* ========================== 调试配置 ========================== */

#define configQUEUE_REGISTRY_SIZE               0               /* 队列注册表大小 */
#define configPRINTF( x )       printf x        /* 用于调试输出 */

/* ========================== 中断优先级配置 ========================== */

/**
 * STM32F407 使用 4 位优先级分组
 * FreeRTOS 使用最高 4 位作为抢占优先级
 * 
 * 对于 Cortex-M4，中断优先级配置如下：
 * - configKERNEL_INTERRUPT_PRIORITY = 0b11000000 (0xC0)  [优先级 3]
 * - configMAX_SYSCALL_INTERRUPT_PRIORITY = 0b10100000 (0xA0) [优先级 2]
 * 
 * 所有 FreeRTOS 兼容的中断必须在这两个值之间
 */

#define configPRIO_BITS                         4               /* STM32F407 有 4 位优先级 */
#define configKERNEL_INTERRUPT_PRIORITY         0xF0            /* 内核中断优先级（最低） */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    0x50            /* 最高 API 调用优先级 */

/* ========================== SysTick 和时钟配置 ========================== */

#define xPortPendSVHandler      PendSV_Handler  /* PendSV 处理函数 */
#define xPortSysTickHandler     SysTick_Handler /* SysTick 处理函数 */
#define vPortSVCHandler         SVC_Handler     /* SVC 处理函数 */

/* ========================== 内存堆配置 ========================== */

#if configUSE_STATIC_ALLOCATION == 1
    extern void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                                StackType_t **ppxIdleTaskStackBuffer,
                                                uint32_t *pulIdleTaskStackSize );

    extern void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                                 StackType_t **ppxTimerTaskStackBuffer,
                                                 uint32_t *pulTimerTaskStackSize );
#endif

/* ========================== 可选钩子函数 ========================== */

#if configUSE_IDLE_HOOK == 1
    extern void vApplicationIdleHook( void );
#endif

#if configUSE_TICK_HOOK == 1
    extern void vApplicationTickHook( void );
#endif

#if configCHECK_FOR_STACK_OVERFLOW > 0
    extern void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName );
#endif

#if configUSE_DAEMON_TASK_DELETION == 1
    extern void vApplicationDaemonTaskStartupHook( void );
#endif

#if configGENERATE_RUN_TIME_STATS == 1
    extern void vConfigureTimerForRunTimeStats( void );
    extern unsigned long ulGetRunTimeCounterValue( void );
#endif

#endif /* FREERTOS_CONFIG_H */
```

</details>

### 关键配置说明

| 配置项 | 值 | 说明 |
|--------|-----|------|
| `configTICK_RATE_HZ` | 1000 | 1ms 时钟周期（标准值） |
| `configCPU_CLOCK_HZ` | 168MHz | STM32F407 主频 |
| `configTOTAL_HEAP_SIZE` | 32KB | 堆大小（根据需要调整） |
| `configMAX_PRIORITIES` | 5 | 5 个优先级 (0=最低, 4=最高) |
| `configMINIMAL_STACK_SIZE` | 128 | 最小栈 128 字 (512B) |
| `configUSE_PREEMPTION` | 1 | 抢占式调度 |
| `configKERNEL_INTERRUPT_PRIORITY` | 0xF0 | 内核中断优先级 |

---

## 中断处理配置

### 修改 stm32f4xx_it.c

现在的 SysTick 处理中既有 KEY_Tick 又有 FreeRTOS，需要合并：

```c
// 在 stm32f4xx_it.c 的顶部添加
#include "FreeRTOS.h"
#include "task.h"
#include "BSP/KEY/key.h"

void SysTick_Handler(void)
{
    HAL_IncTick();
    
    /* FreeRTOS 时钟 - 必须放在 HAL_IncTick() 之后 */
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
    
    /* 按键定时采样 */
    static uint8_t key_tick_count = 0;
    key_tick_count++;
    
    if (key_tick_count >= 5)
    {
        key_tick_count = 0;
        KEY_Tick();
    }
}
```

### 处理中断优先级

在 main.c 中的初始化部分添加：

```c
void vApplicationSetupTimerInterrupt( void )
{
    /* 设置中断优先级分组 - 第一次设置 */
    NVIC_SetPriorityGrouping( 0 );
    
    /* 设置 SysTick 优先级（最低） */
    NVIC_SetPriority( SysTick_IRQn, configKERNEL_INTERRUPT_PRIORITY );
    
    /* 设置 PendSV 优先级（最低） */
    NVIC_SetPriority( PendSV_IRQn, configKERNEL_INTERRUPT_PRIORITY );
    
    /* 设置 SVCall 优先级（比 SysTick 高） */
    NVIC_SetPriority( SVCall_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY );
}
```

---

## 主程序初始化

### 修改 main.c 的初始化流程

**修改前**：
```c
int main(void)
{
    sys_init();          // 系统初始化
    delay_init(168);     // 延迟初始化
    usart_init(115200);  // 串口初始化
    
    // ... 其他初始化 ...
    
    while (1)
    {
        audio_play();    // 主循环
    }
}
```

**修改后**：
```c
#include "FreeRTOS.h"
#include "task.h"

/* 任务句柄 */
TaskHandle_t xAudioPlayTaskHandle = NULL;

/* 音频播放任务 */
void vAudioPlayTask(void *pvParameters)
{
    (void) pvParameters;  // 不使用参数，消除警告
    
    while (1)
    {
        audio_play();     // 播放音乐
    }
}

int main(void)
{
    sys_init();          // 系统初始化
    delay_init(168);     // 延迟初始化
    usart_init(115200);  // 串口初始化
    key_init();          // 按键初始化
    lcd_init();          // LCD 初始化
    sd_init();           // SD 卡初始化
    
    // ... 其他初始化 ...
    
    printf("System initialized. Starting FreeRTOS...\r\n");
    
    /* 创建任务 */
    xTaskCreate(
        vAudioPlayTask,           /* 任务函数 */
        "Audio Play Task",        /* 任务名字 */
        512,                      /* 栈大小（字） */
        NULL,                     /* 任务参数 */
        2,                        /* 优先级（0-4） */
        &xAudioPlayTaskHandle     /* 任务句柄 */
    );
    
    printf("Audio Play Task created!\r\n");
    
    /* 启动调度器 */
    vTaskStartScheduler();
    
    /* 如果代码执行到这里，说明调度器启动失败 */
    printf("ERROR: Scheduler start failed!\r\n");
    
    while (1);
}

/* 处理内存不足的回调 */
void vApplicationMallocFailedHook(void)
{
    printf("ERROR: FreeRTOS malloc failed!\r\n");
    while (1);
}

/* 栈溢出处理 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("ERROR: Stack overflow in task: %s\r\n", pcTaskName);
    while (1);
}
```

---

## 任务创建和管理

### 多任务示例

如果要同时运行多个任务（如音频播放 + 按键监测），可以这样设计：

```c
/* LED 闪烁任务 */
void vLedBlinkTask(void *pvParameters)
{
    (void) pvParameters;
    
    while (1)
    {
        LED0_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));  /* 延迟 500ms */
    }
}

/* 按键监测任务（可选，如果不在播放循环中处理） */
void vKeyDetectTask(void *pvParameters)
{
    (void) pvParameters;
    
    while (1)
    {
        /* 在这里处理按键事件 */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* 音频播放任务 */
void vAudioPlayTask(void *pvParameters)
{
    (void) pvParameters;
    
    while (1)
    {
        audio_play();
        /* audio_play() 是阻塞的，但 FreeRTOS 可以用 SysTick 抢占 */
    }
}

int main(void)
{
    // ... 初始化代码 ...
    
    /* 创建多个任务 */
    xTaskCreate(vLedBlinkTask, "LED Blink", 128, NULL, 1, NULL);
    xTaskCreate(vKeyDetectTask, "Key Detect", 256, NULL, 2, NULL);
    xTaskCreate(vAudioPlayTask, "Audio Play", 512, NULL, 2, NULL);
    
    vTaskStartScheduler();
    
    while (1);
}
```

### 任务优先级说明

```
优先级 4 (最高)  ← 关键实时任务（如硬件中断处理）
优先级 3         ← 重要的实时任务（如音频播放）
优先级 2         ← 一般任务（如按键处理、LED）
优先级 1         ← 后台任务
优先级 0 (最低)  ← Idle 任务（系统自动创建）
```

---

## 现有代码集成

### 关键 API 替换

当使用 FreeRTOS 后，一些延迟和等待方式需要改变：

```c
/* ❌ 旧方式（阻塞式）*/
delay_ms(1000);

/* ✅ 新方式（产生时间片） */
vTaskDelay(pdMS_TO_TICKS(1000));  // 延迟 1000ms，其他任务可运行
```

### 播放循环的改进

原来的播放循环在 `audio_play()` 中是无限的，现在可以改进为：

```c
void audio_play(void)
{
    // ... 初始化代码 ...
    
    while (totwavnum > 0)
    {
        // ... 播放文件 ...
        
        key = audio_play_song(pname);
        
        // ... 处理按键 ...
        
        /* 为其他任务让出时间片 */
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    /* 播放结束后，任务休眠而不是忙轮询 */
    vTaskDelay(pdMS_TO_TICKS(100));
}
```

### 互斥锁示例

如果多个任务需要访问共享资源（如 LCD），使用互斥锁：

```c
/* 全局互斥锁 */
SemaphoreHandle_t xLcdMutex = NULL;

void vAudioPlayTask(void *pvParameters)
{
    while (1)
    {
        /* 请求互斥锁 */
        if (xSemaphoreTake(xLcdMutex, pdMS_TO_TICKS(100)))
        {
            /* 安全地访问 LCD */
            audio_msg_show(totsec, cursec, bitrate);
            
            /* 释放互斥锁 */
            xSemaphoreGive(xLcdMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main(void)
{
    // ... 初始化 ...
    
    /* 创建互斥锁 */
    xLcdMutex = xSemaphoreCreateMutex();
    
    // ... 创建任务 ...
}
```

---

## 测试和调试

### 编译配置

在 Keil 项目中：

1. **添加包含路径**：
   ```
   FreeRTOS\include
   FreeRTOS\portable\ARM_CM4F
   User
   ```

2. **添加源文件到项目**：
   ```
   FreeRTOS\src\tasks.c
   FreeRTOS\src\queue.c
   FreeRTOS\src\timers.c
   FreeRTOS\src\event_groups.c
   FreeRTOS\src\stream_buffer.c
   FreeRTOS\src\heap_4.c
   FreeRTOS\portable\ARM_CM4F\port.c
   ```

3. **编译选项**：
   - C99 或更新版本
   - 优化级别：-O2（推荐）

### 测试代码

```c
/* 简单测试任务 */
void vTestTask(void *pvParameters)
{
    int count = 0;
    
    while (1)
    {
        count++;
        printf("Task running... count = %d\r\n", count);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void)
{
    // ... 初始化 ...
    
    xTaskCreate(vTestTask, "Test", 256, NULL, 1, NULL);
    
    vTaskStartScheduler();
    
    while (1);
}
```

### 调试技巧

1. **查看任务列表**：
   ```c
   char taskList[512];
   vTaskList(taskList);
   printf("%s\r\n", taskList);
   ```

2. **查看堆使用情况**：
   ```c
   printf("Free heap: %d bytes\r\n", xPortGetFreeHeapSize());
   ```

3. **使用断言**：
   ```c
   configASSERT(xHandle != NULL);  /* 如果为 NULL 会断言 */
   ```

---

## 📝 完整的移植检查清单

- [ ] 下载 FreeRTOS 源码
- [ ] 创建文件夹结构 (FreeRTOS/src, include, portable)
- [ ] 复制所有必需的源文件
- [ ] 创建 FreeRTOSConfig.h
- [ ] 修改 stm32f4xx_it.c (SysTick 处理)
- [ ] 修改 main.c (初始化和任务创建)
- [ ] 添加 Keil 项目包含路径
- [ ] 添加源文件到编译列表
- [ ] 编译检查（0 errors, 可能有 warnings）
- [ ] 下载到开发板
- [ ] 串口打印测试
- [ ] 验证音频播放
- [ ] 验证按键响应
- [ ] 查看任务列表和堆信息

---

## 🚨 常见问题

### Q1: 编译错误 "undefined reference to vTaskStartScheduler"
**原因**: 没有添加 tasks.c 到项目
**解决**: 在 Keil 中添加 FreeRTOS/src/tasks.c

### Q2: 音频播放卡顿
**原因**: DMA 中断优先级过低或堆不足
**解决**: 
- 增加堆大小 `configTOTAL_HEAP_SIZE`
- 调整 DMA 中断优先级 > `configMAX_SYSCALL_INTERRUPT_PRIORITY`

### Q3: 栈溢出错误
**原因**: 任务栈大小不足
**解决**: 增加任务栈大小 `xTaskCreate(..., 1024, ...)`

### Q4: FreeRTOS 无法启动
**原因**: SysTick 配置错误或时钟不对
**解决**: 检查 `configCPU_CLOCK_HZ` 和 `configTICK_RATE_HZ`

---

## 📚 参考资源

1. FreeRTOS 官网: https://www.freertos.org/
2. FreeRTOS 文档: https://www.freertos.org/RTOS-Cortex-M-NVIC-priority-levels.html
3. STM32F407 数据表: STM32F4xx Reference Manual
4. ARM Cortex-M4: ARM Cortex-M4 Devices Generic User Guide

