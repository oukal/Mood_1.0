/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-14
 * @brief       template 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 * @notes 
 *
 ****************************************************************************************************
 */

#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "SYSTEM/delay/delay.h"
#include "BSP/LED/led.h"
#include "BSP/KEY/key.h"
#include "BSP/LCD/lcd.h"
#include "./USMART/usmart.h"
#include "./BSP/SDIO/sdio_sdcard.h"
  #include "./TEXT/text.h"
#include "./BSP/ES8388/es8388.h"
#include "./APP/audioplay.h"

#include "./FATFS/source/ff.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

//定义全局的文件系统对象
FATFS fs;
FIL file;
UINT br;
uint8_t read_buf[100];


//定义任务堆栈的内存地址和大小
static const HeapRegion_t xHeapRegions[] =
{
	{(uint8_t *)0x10000000UL, 0x10000},		//CCM		64kByte
	{NULL, 0}
};


/**
 * @brief       获得时间
 * @param       mf  : 内存首地址
 * @retval      时间
 *   @note      时间编码规则如下:
 *              User defined function to give a current time to fatfs module 
 *              31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31)
 *              15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) 
 */
DWORD get_fattime (void)
{
    return 0;
}


/**
 * @brief       通过串口打印SD卡相关信息
 * @param       无
 * @retval      无
 */
void show_sdcard_info(void)
{
    HAL_SD_CardCIDTypeDef sd_card_cid;

    HAL_SD_GetCardCID(&g_sdcard_handler, &sd_card_cid); /* 获取CID */
    get_sd_card_info(&g_sd_card_info_handle);           /* 获取SD卡信息 */

    switch (g_sd_card_info_handle.CardType)
    {
    case CARD_SDSC:
    {
        if (g_sd_card_info_handle.CardVersion == CARD_V1_X)
        {
            printf("Card Type:SDSC V1\r\n");
        }
        else if (g_sd_card_info_handle.CardVersion == CARD_V2_X)
        {
            printf("Card Type:SDSC V2\r\n");
        }
    }
    break;

    case CARD_SDHC_SDXC:
        printf("Card Type:SDHC\r\n");
        break;
    default: break;
    }

    printf("Card ManufacturerID:%d\r\n", sd_card_cid.ManufacturerID);                   /* 制造商ID */
    printf("Card RCA:%d\r\n", g_sd_card_info_handle.RelCardAdd);                        /* 卡相对地址 */
    printf("LogBlockNbr:%d \r\n", (uint32_t)(g_sd_card_info_handle.LogBlockNbr));       /* 显示逻辑块数量 */
    printf("LogBlockSize:%d \r\n", (uint32_t)(g_sd_card_info_handle.LogBlockSize));     /* 显示逻辑块大小 */
    printf("Card Capacity:%d MB\r\n", (uint32_t)SD_TOTAL_SIZE_MB(&g_sdcard_handler));   /* 显示容量 */
    printf("Card BlockSize:%d\r\n\r\n", g_sd_card_info_handle.BlockSize);               /* 显示块大小 */
}



/* LED状态设置函数 */
void led_set(uint8_t sta)
{
    LED1(sta);
}

/* 函数参数调用测试函数 */
void test_fun(void(*ledset)(uint8_t), uint8_t sta)
{
    ledset(sta);
}


static void vLed_Task(void *pvParameters)
{
	static portTickType xLastWakeTime;
	

	xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		LED1_TOGGLE();
		vTaskDelayUntil(&xLastWakeTime, 1000/portTICK_RATE_MS);
	}
}



static void vAudio_Task(void *pvParameters)
{

	
	audio_play();
	vTaskDelay(10);
	for (;;)
	{
	
	}
}


int main(void)
{ 


	//必须选择为组4，中断均为抢占优先级
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	//屏蔽中断，防止系统没有启动，而在中断里调用了系统的函数
	__set_PRIMASK(1);


/*---------------------------初始化-------------------------*/
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(336, 8, 2, 7); /* 设置时钟,168Mhz */
    delay_init(168);                    /* 延时初始化 */
	usart_init(115200);                     /* 串口初始化为115200 */
 	usmart_dev.init(84);                    /* USMART初始化 */
    led_init();                         /* 初始化LED */
    key_init();                         /* 初始化KEY */
	lcd_init();                             /* 初始化LCD */
	LED0(0);
	
	     while (sd_init())                       /* 检测SD卡 */
    {
        lcd_show_string(30, 50, 200, 16, 16, "SD Card Failed!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 200 + 30, 50 + 16, WHITE);
        delay_ms(200);
    }
    f_mount(&fs, "0:", 1);        /* 挂载SD卡 */

    
    es8388_init();              /* ES8388初始化 */
    es8388_adda_cfg(1, 0);      /* 开启DAC关闭ADC */
    es8388_output_cfg(1, 1);    /* DAC选择通道输出 */
    es8388_hpvol_set(25);       /* 设置耳机音量 */
    es8388_spkvol_set(0); /* 设置喇叭音量 */
    
    text_show_string(30, 30, 200, 16, "正点原子STM32开发板", 16, 0, RED);
    text_show_string(30, 50, 200, 16, "音乐播放器实验", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "正点原子@ALIENTEK", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "2021年11月16日", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY0:NEXT   KEY2:PREV", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "KEY_UP:PAUSE/PLAY", 16, 0, RED);
    
	
		xTaskCreate(vAudio_Task, \
				"Audio", \
				1024, \
				NULL, \
				2, \
				NULL);
	xTaskCreate(vLed_Task, \
				"Led", \
				configMINIMAL_STACK_SIZE, \
				NULL, \
				2, \
				NULL);
	
	
	
	vTaskStartScheduler();
	
	
	
    while (1)
    {
        audio_play();           /* 播放音乐 */
    }

 }

 
//----------------------------------------------------------------------
//FreeRTOS 钩子函数
void vApplicationMallocFailedHook(void)
{
//	log_record("\r\n#[error_msg]: Malloc failed!\r\n");
//	__set_PRIMASK(1);
	for (;;)
	{
		;
	}
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
//	log_record("\r\n#[error_msg]: Stack overflow! Handle: %x, Task: %s\r\n", pxTask, pcTaskName);
//	__set_PRIMASK(1);
	for (;;)
	{
		;
	}
}
