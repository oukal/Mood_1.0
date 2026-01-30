# FreeRTOS 移植快速开始 (5 步完成)

## 🚀 5 分钟快速指南

### 第 1 步: 下载 FreeRTOS (2 分钟)

```bash
# 方案 A: 从官网下载
# 访问 https://www.freertos.org/download-freertos-plus.html
# 下载最新版本 → 解压

# 方案 B: 用 Git
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
cd FreeRTOS-Kernel
git checkout V10.4.3
```

### 第 2 步: 复制文件到项目 (2 分钟)

在 `d:\desktop\Mood_1.0\` 下创建文件夹结构：

```
Mood_1.0/
├── FreeRTOS/
│   ├── src/
│   │   ├── tasks.c
│   │   ├── queue.c
│   │   ├── timers.c
│   │   ├── event_groups.c
│   │   ├── stream_buffer.c
│   │   ├── croutine.c
│   │   └── heap_4.c
│   ├── include/
│   │   ├── FreeRTOS.h
│   │   ├── task.h
│   │   ├── queue.h
│   │   └── ... (所有头文件)
│   └── portable/
│       └── ARM_CM4F/
│           ├── port.c
│           └── portmacro.h
```

从官方下载的 FreeRTOS 包中复制：
- `kernel/tasks.c` → `FreeRTOS/src/tasks.c`
- `kernel/queue.c` → `FreeRTOS/src/queue.c`
- ... (其他文件)
- `portable/GCC/ARM_CM4F/port.c` → `FreeRTOS/portable/ARM_CM4F/port.c`
- `include/*.h` → `FreeRTOS/include/`

### 第 3 步: 创建配置文件 (1 分钟)

创建 `d:\desktop\Mood_1.0\User\FreeRTOSConfig.h`，内容如下：

```c
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* 基础配置 */
#define configUSE_PREEMPTION                    1
#define configCPU_CLOCK_HZ                      (168000000UL)
#define configTICK_RATE_HZ                      (1000UL)
#define configMAX_PRIORITIES                    (5)
#define configMINIMAL_STACK_SIZE                (128)
#define configTOTAL_HEAP_SIZE                   (32 * 1024)
#define configMAX_TASK_NAME_LEN                 (16)
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0

/* 功能开关 */
#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TIMERS                        1
#define configUSE_EVENT_GROUPS                  1
#define configUSE_STREAM_BUFFERS                1
#define configUSE_TASK_NOTIFICATIONS            1

/* 中断优先级 */
#define configKERNEL_INTERRUPT_PRIORITY         0xF0
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    0x50

/* 钩子函数 */
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0

/* 定时器 */
#define configTIMER_TASK_PRIORITY               4
#define configTIMER_QUEUE_LENGTH                10

#endif
```

### 第 4 步: 修改 Keil 项目 (1 分钟)

在 Keil MDK 中：

1. **Project → Options → C/C++ → Include Paths** 添加：
   ```
   FreeRTOS\include
   FreeRTOS\portable\ARM_CM4F
   User
   ```

2. **在项目树中添加文件** (右键 Project → Add Group → Add Files):
   ```
   FreeRTOS/src/tasks.c
   FreeRTOS/src/queue.c
   FreeRTOS/src/timers.c
   FreeRTOS/src/event_groups.c
   FreeRTOS/src/stream_buffer.c
   FreeRTOS/src/croutine.c
   FreeRTOS/src/heap_4.c
   FreeRTOS/portable/ARM_CM4F/port.c
   ```

### 第 5 步: 修改代码 (1 分钟)

#### 修改 stm32f4xx_it.c

```c
#include "FreeRTOS.h"
#include "task.h"
#include "BSP/KEY/key.h"

void SysTick_Handler(void)
{
    HAL_IncTick();
    
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
    
    static uint8_t key_tick_count = 0;
    key_tick_count++;
    if (key_tick_count >= 5)
    {
        key_tick_count = 0;
        KEY_Tick();
    }
}
```

#### 修改 main.c

```c
#include "FreeRTOS.h"
#include "task.h"

void vAudioPlayTask(void *pvParameters)
{
    while (1)
    {
        audio_play();
    }
}

int main(void)
{
    sys_init();
    delay_init(168);
    usart_init(115200);
    key_init();
    lcd_init();
    sd_init();
    
    /* 其他初始化... */
    
    /* 创建任务 */
    xTaskCreate(vAudioPlayTask, "Audio", 512, NULL, 2, NULL);
    
    /* 启动调度器 */
    vTaskStartScheduler();
    
    /* 不应该到达这里 */
    while (1);
}

/* 错误处理 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("Stack overflow in task: %s\r\n", pcTaskName);
    while (1);
}
```

---

## ✅ 编译和测试

### 编译
```
Project → Build → 按 F7
```

### 检查编译结果
```
Build output should show:
✅ 0 errors
⚠️ warnings 可以忽略（通常是有关注释的）
```

### 下载和运行
```
1. 插入开发板
2. Project → Download → 按 Ctrl+F5
3. 应该看到串口打印启动信息
4. 音频播放功能应该正常工作
5. 按键响应应该更快
```

---

## 🧪 简单测试
    f
添加一个 LED 闪烁任务来验证 FreeRTOS 工作：

```c
void vLedBlinkTask(void *pvParameters)
{
    while (1)
    {
        LED0_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));  /* 每 500ms 闪烁一次 */
    }
}

int main(void)
{
    // ... 初始化 ...
    
    xTaskCreate(vLedBlinkTask, "LED", 128, NULL, 1, NULL);
    xTaskCreate(vAudioPlayTask, "Audio", 512, NULL, 2, NULL);
    
    vTaskStartScheduler();
    
    while (1);
}
```

如果 LED 在闪烁，而音频也在播放，说明 FreeRTOS 运行成功！

---

## 📊 移植前后对比

| 特性 | 移植前 | 移植后 |
|------|--------|--------|
| **任务管理** | 单个 main loop | 多个独立任务 |
| **优先级** | 无 | 5 个级别可配置 |
| **资源管理** | 手动控制 | 自动调度 |
| **延迟方式** | 阻塞式 delay | 产生时间片 vTaskDelay |
| **响应性** | 依赖顺序执行 | 中断驱动 |
| **可扩展性** | 困难 | 容易 |

---

## 🆘 快速排查

| 问题 | 原因 | 解决 |
|------|------|------|
| 编译失败 | 缺少源文件 | 检查所有 .c 文件是否添加到项目 |
| 无法启动 | 时钟配置错 | 确认 CPU_CLOCK_HZ 为 168MHz |
| 音频卡顿 | 堆不足 | 增加 configTOTAL_HEAP_SIZE |
| LED 不闪烁 | 任务未运行 | 检查 vTaskStartScheduler() 是否被调用 |
| 栈溢出 | 栈太小 | 增加 xTaskCreate 的第 3 个参数 |

---

## 💾 下一步

移植完成后，你可以：

1. **添加更多任务**：
   - 按键处理任务
   - LCD 显示任务
   - SD 卡管理任务

2. **使用 FreeRTOS 特性**：
   - 队列通信
   - 信号量同步
   - 互斥锁保护

3. **性能优化**：
   - 查看任务统计信息
   - 调整优先级
   - 监控堆和栈使用

---

## 📞 需要帮助?

遇到问题时检查：
1. `FreeRTOSConfig.h` 中的时钟配置
2. `portmacro.h` 中的优先级设置
3. 串口打印输出
4. LED 状态（确认系统在运行）

