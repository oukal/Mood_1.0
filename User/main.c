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

//定义全局的文件系统对象
FATFS fs;
FIL file;
UINT br;
uint8_t read_buf[100];

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

///**
// * @brief       测试SD卡的读取
// *   @note      从secaddr地址开始,读取seccnt个扇区的数据
// * @param       secaddr : 扇区地址
// * @param       seccnt  : 扇区数
// * @retval      无
// */
//void sd_test_read(uint32_t secaddr, uint32_t seccnt)
//{
//    uint32_t i;
//    //uint8_t *buf;
//    uint8_t sta = 0;
//    uint8_t buf[512];
//    //buf = mymalloc(SRAMIN, seccnt * 512);       /* 申请内存,从SRAM申请内存 */
//    sta = sd_read_disk(buf, secaddr, seccnt);   /* 读取secaddr扇区开始的内容 */

//    if (sta == 0)
//    {
//        lcd_show_string(30, 170, 200, 16, 16, "USART1 Sending Data...", BLUE);
//        printf("SECTOR %d DATA:\r\n", secaddr);

//        for (i = 0; i < seccnt * 512; i++)
//        {
//            printf("%x ", buf[i]);              /* 打印secaddr开始的扇区数据 */
//        }

//        printf("\r\nDATA ENDED\r\n");
//        lcd_show_string(30, 170, 200, 16, 16, "USART1 Send Data Over!", BLUE);
//    }
//    else
//    {
//        printf("err:%d\r\n", sta);
//        lcd_show_string(30, 170, 200, 16, 16, "SD read Failure!      ", BLUE);
//    }

//    //myfree(SRAMIN, buf); /* 释放内存 */
//}

///**
// * @brief       测试SD卡的写入
// *   @note      从secaddr地址开始,写入seccnt个扇区的数据
// *              慎用!! 最好写全是0XFF的扇区,否则可能损坏SD卡.
// *
// * @param       secaddr : 扇区地址
// * @param       seccnt  : 扇区数
// * @retval      无
// */
//void sd_test_write(uint32_t secaddr, uint32_t seccnt)
//{
//    uint32_t i;
//    //uint8_t *buf;
//    uint8_t sta = 0;
//    uint8_t buf_w[512];
//    //buf = mymalloc(SRAMIN, seccnt * 512);       /* 从SRAM申请内存 */

//    for (i = 0; i < seccnt * 512; i++)          /* 初始化写入的数据,是3的倍数. */
//    {
//        buf_w[i] = i * 3;
//    }

//    sta = sd_write_disk(buf_w, secaddr, seccnt);  /* 从secaddr扇区开始写入seccnt个扇区内容 */

//    if (sta == 0)
//    {
//        printf("Write over!\r\n");
//    }
//    else
//    {
//        printf("err:%d\r\n", sta);
//    }

//    //myfree(SRAMIN, buf); /* 释放内存 */
//}



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


int main(void)
{ 
//	uint8_t len = 0;
//    uint8_t lcd_id[12];
//	uint8_t key=0;
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
//    exfuns_init();                          /* 为fatfs相关变量申请内存 */

//    f_mount(fs[1], "1:", 1);                /* 挂载FLASH */

//    while (fonts_init())                    /* 检查字库 */
//    {
//        lcd_show_string(30, 50, 200, 16, 16, "Font Error!", RED);
//        delay_ms(200);
//        lcd_fill(30, 50, 200 + 30, 50 + 16, WHITE);
//        delay_ms(200);
//    }
    
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
    
    while (1)
    {
        audio_play();           /* 播放音乐 */
    }
	
	
    //  测试sd卡在 fatfs挂载
//	 uint8_t res;
//	res=f_mount(&fs, "0:", 1);        /* 挂载SD卡 */
//	if(res==0)
//	{       printf("SD card mounted\r\n");
//		    printf("totol size : %llu MB \r\n", (uint64_t) ((fs.n_fatent)-2)*fs.csize/2/1024);
//		
//		
//        // 1. 尝试“写”文件 (如果没有就创建，如果有就覆盖)
//			printf("Start Write Test...\r\n");
//			res = f_open(&file, "0:test.txt", FA_CREATE_ALWAYS | FA_WRITE);
//			if (res == FR_OK)
//			{
//				char write_data[] = "MoodFlow Write Test OK!";
//				UINT bw;
//				
//				// 写入数据
//				f_write(&file, write_data, sizeof(write_data)-1, &bw);
//				printf("Write OK! Written %d bytes.\r\n", bw);
//				
//				f_close(&file); // 必须关闭，数据才会真正刷入SD卡
//			}
//			else
//			{
//				printf("Write Open Failed! Error: %d\r\n", res);
//			}

//			// 2. 尝试“读”文件 (验证刚才写进去没)
//			printf("Start Read Test...\r\n");
//			res = f_open(&file, "0:test.txt", FA_READ);
//			if (res == FR_OK)
//			{
//				// 读取
//				f_read(&file, read_buf, 50, &br);
//				read_buf[br] = 0; // 封口
//				printf("Read Content: %s\r\n", read_buf);
//				f_close(&file);
//			}
//		}
	
	
	

	
	
	
	
//    g_point_color = RED;
//    sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id);  /* 将LCD ID打印到lcd_id数组 */
//    lcd_clear(GREEN);
//	//lcd_show_string(10, 40, 240, 32, 32, "STM32", RED);
//    //lcd_show_string(10, 80, 240, 24, 24, "TFTLCD TEST", RED);
//    //lcd_show_string(10, 110, 240, 16, 16, "ATOM@ALIENTEK", RED);
//    lcd_show_string(10, 130, 240, 16, 16, (char *)lcd_id, RED); /* 显示LCD ID */
//    delay_ms(1000);
//	printf("hello mood.v1\r\n");
//	
//    //sdcard
//    lcd_show_string(30,  50, 200, 16, 16, "STM32", RED);
//    lcd_show_string(30,  70, 200, 16, 16, "SD TEST", RED);
//    lcd_show_string(30,  90, 200, 16, 16, "ATOM@ALIENTEK", RED);
//    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Read Sector 0", RED);

////    while (sd_init())   /* 检测不到SD卡 */
////    {
////        lcd_show_string(30, 130, 200, 16, 16, "   SD Card Error!", RED);
////        delay_ms(500);
////        lcd_show_string(10, 130, 200, 16, 16, "   Please Check! ", RED);
////        delay_ms(500);
////        LED0_TOGGLE();  /* 红灯闪烁 */
////    }

//    show_sdcard_info(); /* 打印SD卡相关信息 */

//    /* 检测SD卡成功 */
//    lcd_show_string(30, 130, 200, 16, 16, "SD Card OK    ", BLUE);
//    lcd_show_string(30, 150, 200, 16, 16, "SD Card Size:     MB", BLUE);
//    lcd_show_num(30 + 13 * 8, 150, SD_TOTAL_SIZE_MB(&g_sdcard_handler), 5, 16, BLUE); /* 显示SD卡容量 */
//    
//	while(1)
//    {
//         if (g_usart_rx_sta & 0x8000)         /* 接收到了数据? */
//        {
//            len = g_usart_rx_sta & 0x3fff;  /* 得到此次接收到的数据长度 */
//            printf("\r\n您发送的消息为:\r\n");

//            HAL_UART_Transmit(&g_uart1_handle,(uint8_t*)g_usart_rx_buf,len,1000);    /* 发送接收到的数据 */
//            while(__HAL_UART_GET_FLAG(&g_uart1_handle,UART_FLAG_TC)!=SET);           /* 等待发送结束 */
//            printf("\r\n\r\n");             /* 插入换行 */
//            g_usart_rx_sta = 0;
//        }
//		
//		KEY_Tick();
//        key = KEY_GetKeyDown(); /* 得到键值 */
//		 if (key)
//		 {
//		 switch (key)
//		 {
//			 case KEY_WAKE_PRES: /* 控制蜂鸣器 */
//				//sd_test_read(0,1);  /* 从0扇区读取1*512字节的内容 *///BEEP_TOGGLE(); /* BEEP 状态取反 */
//			 break;
//			 case KEY0_PRES: /* 控制 LED0(RED)翻转 */
//				LED0_TOGGLE(); /* LED0 状态取反 */
//			 break;
//			 case KEY1_PRES: /* 控制 LED1(GREEN)翻转 */
//				LED1_TOGGLE(); /* LED1 状态取反 */
//			 break;
//			 case KEY2_PRES: /* 同时控制 LED0, LED1 翻转 */
//				LED0_TOGGLE(); /* LED0 状态取反 */
//				LED1_TOGGLE(); /* LED1 状态取反 */
//			 break;
//			 default : 
//			 break;
//         }
//        }
//    
//     }
	
 }

 
