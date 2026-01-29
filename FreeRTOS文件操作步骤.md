# FreeRTOS 文件操作详细步骤

## 📂 完整文件结构

```
d:\desktop\Mood_1.0\
├── Drivers/
├── Middlewares/
├── Projects/
│   └── MDK-ARM/
│       └── atk_f407.uvprojx (Keil 项目)
├── User/
│   ├── main.c (修改)
│   ├── stm32f4xx_it.c (修改)
│   ├── stm32f4xx_hal_conf.h
│   ├── FreeRTOSConfig.h (新建)
│   └── ...
└── FreeRTOS/  (新建)
    ├── src/
    │   ├── tasks.c
    │   ├── queue.c
    │   ├── timers.c
    │   ├── event_groups.c
    │   ├── stream_buffer.c
    │   ├── croutine.c
    │   └── heap_4.c
    ├── include/
    │   ├── FreeRTOS.h
    │   ├── task.h
    │   ├── queue.h
    │   ├── timers.h
    │   ├── event_groups.h
    │   ├── stream_buffer.h
    │   ├── croutine.h
    │   ├── list.h
    │   ├── semphr.h
    │   ├── projdefs.h
    │   ├── portable.h
    │   ├── mpu_wrappers.h
    │   └── ...其他头文件
    └── portable/
        └── ARM_CM4F/
            ├── port.c
            └── portmacro.h
```

---

## 🔧 逐步操作指南

### 步骤 1: 下载 FreeRTOS 源码

#### 方法 A: 官网下载 (推荐新手)

```
1. 打开浏览器
2. 访问 https://www.freertos.org/
3. 点击 "DOWNLOAD"
4. 选择最新版本（如 FreeRTOS V10.4.3）
5. 下载 ZIP 文件到本地
6. 用 7-Zip 或 WinRAR 解压
```

#### 方法 B: Git 克隆 (推荐开发者)

```powershell
# 打开 PowerShell，进入项目目录
cd d:\desktop\Mood_1.0

# 克隆 FreeRTOS 仓库
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS

# 进入目录
cd FreeRTOS

# 检出稳定版本
git checkout V10.4.3
```

### 步骤 2: 创建 FreeRTOS 本地目录结构

在 `d:\desktop\Mood_1.0\` 中创建以下文件夹：

```powershell
# 使用 PowerShell 创建文件夹结构
New-Item -ItemType Directory -Path "FreeRTOS\src" -Force
New-Item -ItemType Directory -Path "FreeRTOS\include" -Force
New-Item -ItemType Directory -Path "FreeRTOS\portable\ARM_CM4F" -Force
```

### 步骤 3: 复制核心源文件

从官方 FreeRTOS 包中复制文件：

#### 3.1 复制 src 文件夹内的文件

```
官方路径: FreeRTOS/kernel/
复制以下文件到: d:\desktop\Mood_1.0\FreeRTOS\src\

□ tasks.c               (核心任务管理)
□ queue.c              (队列管理)
□ timers.c             (软件定时器)
□ event_groups.c       (事件组)
□ stream_buffer.c      (流缓冲)
□ croutine.c           (协程，可选)
```

#### 3.2 复制 portable 文件

```
官方路径: FreeRTOS/portable/GCC/ARM_CM4F/
复制以下文件到: d:\desktop\Mood_1.0\FreeRTOS\portable\ARM_CM4F\

□ port.c
□ portmacro.h
```

#### 3.3 复制堆内存管理文件

```
官方路径: FreeRTOS/portable/MemMang/
复制以下文件到: d:\desktop\Mood_1.0\FreeRTOS\src\

□ heap_4.c             (推荐，最好用的堆)
```

**关于堆的选择**:
- `heap_1.c`: 内存只能分配不能释放（用于简单系统）
- `heap_2.c`: 支持释放但有碎片（已废弃）
- `heap_3.c`: 用 malloc/free（简单但较慢）
- `heap_4.c`: 支持多个内存区域，推荐使用 ✅
- `heap_5.c`: 最复杂，多块 RAM 应用

#### 3.4 复制所有头文件

```
官方路径: FreeRTOS/include/
复制全部 *.h 文件到: d:\desktop\Mood_1.0\FreeRTOS\include\

关键文件:
□ FreeRTOS.h           (主配置文件)
□ task.h               (任务 API)
□ queue.h              (队列 API)
□ timers.h             (定时器 API)
□ event_groups.h       (事件组 API)
□ stream_buffer.h      (流缓冲 API)
□ croutine.h           (协程 API)
□ list.h               (内部链表)
□ semphr.h             (信号量宏)
□ projdefs.h           (项目定义)
□ portable.h           (可移植层)
□ mpu_wrappers.h       (MPU 支持)
□ ...其他头文件
```

### 步骤 4: 创建 FreeRTOSConfig.h

在 `d:\desktop\Mood_1.0\User\` 中创建新文件 `FreeRTOSConfig.h`：

```c
/*
 * FreeRTOS Kernel V10.4.3
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * 应用程序专有定义
 * 这些定义应在构建系统中定义，但为了简便，我们在这里编辑
 *----------------------------------------------------------*/

/* 内核配置 */
#define configUSE_PREEMPTION                    1           /* 使用抢占式调度 */
#define configUSE_IDLE_HOOK                     0           /* 使用 Idle 钩子 */
#define configUSE_TICK_HOOK                     0           /* 使用 Tick 钩子 */
#define configCPU_CLOCK_HZ                      (168000000UL)  /* CPU 频率 168MHz */
#define configTICK_RATE_HZ                      (1000UL)    /* 时钟周期 1ms */
#define configMAX_PRIORITIES                    (5)         /* 优先级数 0-4 */
#define configMINIMAL_STACK_SIZE                (128)       /* 最小栈 128 字 */
#define configTOTAL_HEAP_SIZE                   (32 * 1024) /* 总堆 32KB */
#define configMAX_TASK_NAME_LEN                 (16)        /* 任务名长度 */
#define configUSE_TRACE_FACILITY                1           /* 使用追踪设施 */
#define configUSE_16_BIT_TICKS                  0           /* 32 位 Tick */

/* 功能使能 */
#define configUSE_MUTEXES                       1           /* 互斥锁 */
#define configUSE_RECURSIVE_MUTEXES             1           /* 递归互斥锁 */
#define configUSE_COUNTING_SEMAPHORES           1           /* 计数信号量 */
#define configUSE_QUEUE_SETS                    0           /* 队列集合 */
#define configUSE_TASK_NOTIFICATIONS            1           /* 任务通知 */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1           /* 通知数组大小 */

/* 定时器配置 */
#define configUSE_TIMERS                        1           /* 启用软件定时器 */
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)  /* 定时器优先级 */
#define configTIMER_QUEUE_LENGTH                10          /* 定时器队列长度 */
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

/* 事件组配置 */
#define configUSE_EVENT_GROUPS                  1           /* 启用事件组 */

/* 流缓冲配置 */
#define configUSE_STREAM_BUFFERS                1           /* 启用流缓冲 */

/* 协程配置 */
#define configUSE_CO_ROUTINES                   0           /* 不使用协程 */
#define configMAX_CO_ROUTINE_PRIORITIES         (2)         /* 协程优先级 */

/* 内存分配配置 */
#define configSUPPORT_DYNAMIC_ALLOCATION        1           /* 支持动态分配 */
#define configSUPPORT_STATIC_ALLOCATION         0           /* 不支持静态分配 */

/* 栈溢出检查 */
#define configCHECK_FOR_STACK_OVERFLOW          2           /* 检查等级 2 */
#define configRECORD_STACK_HIGH_ADDRESS         1           /* 记录栈高地址 */

/* 中断优先级配置 - 重要！ */
#define configPRIO_BITS                         4           /* STM32F4 有 4 位优先级 */
#define configKERNEL_INTERRUPT_PRIORITY         0xF0        /* 内核中断优先级（最低） */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    0x50        /* 最高 API 调用优先级 */

/* 钩子函数 */
#define configASSERT( x ) if((x) == 0) { taskDISABLE_INTERRUPTS(); for( ;; ); }
#define configQUEUE_REGISTRY_SIZE               0

#endif /* FREERTOS_CONFIG_H */
```

### 步骤 5: 修改 Keil 项目

打开 `d:\desktop\Mood_1.0\Projects\MDK-ARM\atk_f407.uvprojx`：

#### 5.1 添加包含路径

```
Project → Options → C/C++ → Include Paths

添加以下路径（每行一个）:
../FreeRTOS/include
../FreeRTOS/portable/ARM_CM4F
../User
```

#### 5.2 添加源文件到项目

在 Keil 左侧项目树中：

```
1. 右键点击项目名 → Add Group → 输入 "FreeRTOS" → OK
2. 在新建的 FreeRTOS 组下，右键 → Add Files to Group
3. 浏览到 FreeRTOS/src，选择以下文件：
   - tasks.c ✓
   - queue.c ✓
   - timers.c ✓
   - event_groups.c ✓
   - stream_buffer.c ✓
   - croutine.c ✓
   - heap_4.c ✓
4. 按 Ctrl 多选，然后点击 "Add"

5. 继续添加 portable 文件：
   右键 FreeRTOS 组 → Add Files to Group
   浏览到 FreeRTOS/portable/ARM_CM4F/
   选择 port.c ✓
   点击 "Add"
```

完成后项目树看起来应该像这样：
```
Project
├── User
│   ├── main.c
│   ├── stm32f4xx_it.c
│   └── ...
├── Drivers
│   └── ...
├── Middlewares
│   └── ...
└── FreeRTOS (新增)
    ├── tasks.c
    ├── queue.c
    ├── timers.c
    ├── event_groups.c
    ├── stream_buffer.c
    ├── croutine.c
    ├── heap_4.c
    └── port.c
```

### 步骤 6: 修改源代码

#### 6.1 修改 stm32f4xx_it.c

在文件顶部添加：
```c
#include "FreeRTOS.h"
#include "task.h"
#include "BSP/KEY/key.h"
```

找到 `SysTick_Handler` 函数，替换为：
```c
void SysTick_Handler(void)
{
  HAL_IncTick();
  
  /* FreeRTOS 时钟处理 */
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

#### 6.2 修改 main.c

在文件顶部添加：
```c
#include "FreeRTOS.h"
#include "task.h"
```

创建任务函数（在 main 之前）：
```c
/* 音频播放任务 */
void vAudioPlayTask(void *pvParameters)
{
    (void)pvParameters;  /* 消除编译警告 */
    
    while (1)
    {
        audio_play();    /* 播放音乐 */
    }
}

/* 可选：LED 闪烁任务（用于测试） */
void vLedBlinkTask(void *pvParameters)
{
    (void)pvParameters;
    
    while (1)
    {
        LED0_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

修改 main 函数：
```c
int main(void)
{
    /* 硬件初始化 */
    sys_init();
    delay_init(168);
    usart_init(115200);
    key_init();
    lcd_init();
    sd_init();
    es8388_init();
    es8388_adda_cfg(1, 0);
    es8388_output_cfg(1, 1);
    es8388_hpvol_set(25);
    es8388_spkvol_set(0);
    
    /* 挂载 SD 卡 */
    f_mount(&fs, "0:", 1);
    
    /* 显示信息 */
    text_show_string(30, 30, 200, 16, "STM32 Audio Player", 16, 0, RED);
    text_show_string(30, 50, 200, 16, "FreeRTOS Version", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "KEY0:NEXT KEY2:PREV", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "KEY1:PAUSE WAKE:REPLAY", 16, 0, RED);
    
    printf("System initialized. Starting FreeRTOS...\r\n");
    
    /* 创建任务 */
    xTaskCreate(
        vAudioPlayTask,           /* 任务函数 */
        "AudioPlay",              /* 任务名 */
        512,                      /* 栈大小（字） */
        NULL,                     /* 参数 */
        2,                        /* 优先级 */
        NULL                      /* 任务句柄 */
    );
    
    printf("Tasks created. Starting scheduler...\r\n");
    
    /* 启动 FreeRTOS 调度器 */
    vTaskStartScheduler();
    
    /* 如果代码到这里，说明调度器启动失败 */
    printf("ERROR: Scheduler failed to start!\r\n");
    while (1);
}

/* 错误处理钩子 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("Stack overflow in task: %s\r\n", pcTaskName);
    printf("Free heap: %u bytes\r\n", xPortGetFreeHeapSize());
    while (1);
}

void vApplicationMallocFailedHook(void)
{
    printf("Malloc failed!\r\n");
    while (1);
}
```

### 步骤 7: 编译和测试

```
1. 按 F7 或 Project → Build 编译项目
2. 检查编译结果：应该是 0 errors
3. 按 Ctrl+F5 下载并调试
4. 打开串口监视器（115200, 8N1）
5. 应该看到打印输出：
   - "System initialized. Starting FreeRTOS..."
   - "Tasks created. Starting scheduler..."
6. 音频应该开始播放
7. 按键应该快速响应
```

---

## ✅ 验证清单

- [ ] FreeRTOS 文件夹结构创建正确
- [ ] 所有源文件都复制到 src 和 portable 文件夹
- [ ] 所有头文件都复制到 include 文件夹
- [ ] FreeRTOSConfig.h 已创建在 User 目录
- [ ] Keil 项目的包含路径已添加
- [ ] 所有 FreeRTOS 源文件都添加到项目
- [ ] stm32f4xx_it.c 已修改
- [ ] main.c 已修改
- [ ] 编译无错误（可能有 warnings，没关系）
- [ ] 代码已下载到开发板
- [ ] 串口看到启动信息
- [ ] 音频播放正常
- [ ] 按键响应迅速

