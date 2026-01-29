/**
 ****************************************************************************************************
 * @file        led.h
 * @author      ouka
 * @version     V1.0
 * @date        2025-11-12
 * @brief       LED 驱动代码
 *@notes LED0 PF9- 引脚0—light
         LED1 PF10-引脚0—light
          *
 * 修改说明
 * V1.0
 * 第一次发布
 ****************************************************************************************************
 */

#ifndef __LED_H
#define __LED_H

#include "./SYSTEM/sys/sys.h"

//notes 宏定义：函数 多语句使用do {}while()
/******************************************************************************************/
/* 引脚 定义 */

#define LED0_GPIO_PORT                  GPIOF
#define LED0_GPIO_PIN                   GPIO_PIN_9
#define LED0_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOF_CLK_ENABLE(); }while(0)             /* PF口时钟使能 */

#define LED1_GPIO_PORT                  GPIOF
#define LED1_GPIO_PIN                   GPIO_PIN_10
#define LED1_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOF_CLK_ENABLE(); }while(0)             /* PF口时钟使能 */

/******************************************************************************************/
//逗号表达式 x？A:B LED(0)输出低电平
/* LED端口定义 */
#define LED0(x)   do{ x ? \
                      HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_GPIO_PIN, GPIO_PIN_RESET); \
                  }while(0)       /* LED0 = RED */

#define LED1(x)   do{ x ? \
                      HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_RESET); \
                  }while(0)       /* LED1 = GREEN */

/* LED取反定义 */
#define LED0_TOGGLE()    do{ HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_GPIO_PIN); }while(0)       /* LED0 = !LED0 */
#define LED1_TOGGLE()    do{ HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_GPIO_PIN); }while(0)       /* LED1 = !LED1 */

/******************************************************************************************/
/* 外部接口函数*/
void led_init(void);                                                                            /* 初始化 */

#endif
