/**
 ****************************************************************************************************
 * @file        text.c
 * @author      正点原子团队(ALIENTEK) -> Modified by User
 * @version     V1.2
 * @date        2026-01-13
 * @brief       汉字显示 代码 (SD卡直读版，无NORFLASH，无MALLOC)
 ****************************************************************************************************
 */

#include "string.h"
#include "./TEXT/text.h"
#include "./BSP/LCD/lcd.h"
#include "./FATFS/source/ff.h"      /* 添加 FatFS 头文件 */
/* #include "./MALLOC/malloc.h" */      /* 删除 */
/* #include "./BSP/NORFLASH/norflash.h" */ /* 删除 */

/* 
 * 全局静态变量：用于字库文件常开优化
 * 作用：在播放音乐等高实时性场景下，避免重复 open/close 文件造成的卡顿
 */
static FIL     s_font_file;         /* 字库文件句柄 */
static uint8_t s_is_font_open = 0;  /* 标记文件是否处于常开状态 */
static uint8_t s_current_size = 0;  /* 记录当前打开的是哪种大小的字库 */

/**
 * @brief       开启字库加速模式 (建议在音乐播放循环前调用)
 * @param       size : 字体大小 (12/16/24)
 * @retval      0:成功, 1:失败
 */
uint8_t text_font_file_open(uint8_t size)
{
    char *path;
    
    /* 如果已经打开且大小一致，直接返回成功 */
    if (s_is_font_open && s_current_size == size) return 0;
    
    /* 如果打开了其他大小的，先关闭 */
    if (s_is_font_open) f_close(&s_font_file);
    
    switch (size)
    {
        case 12: path = "/SYSTEM/FONT/GBK12.FON"; break;
        case 16: path = "/SYSTEM/FONT/GBK16.FON"; break;
        case 24: path = "/SYSTEM/FONT/GBK24.FON"; break;
        default: return 1;
    }
    
    if (f_open(&s_font_file, path, FA_READ) == FR_OK)
    {
        s_is_font_open = 1;
        s_current_size = size;
        return 0;
    }
    return 1;
}

/**
 * @brief       关闭字库加速模式 (建议在音乐播放结束后调用)
 * @retval      无
 */
void text_font_file_close(void)
{
    if (s_is_font_open)
    {
        f_close(&s_font_file);
        s_is_font_open = 0;
        s_current_size = 0;
    }
}

/**
 * @brief       获取汉字点阵数据
 * @param       code  : 当前汉字编码(GBK码)
 * @param       mat   : 当前汉字点阵数据存放地址
 * @param       size  : 字体大小
 * @retval      无
 */
static void text_get_hz_mat(unsigned char *code, unsigned char *mat, uint8_t size)
{
    unsigned char qh, ql;
    unsigned char i;
    unsigned long foffset;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size); /* 得到字体一个字符对应点阵集所占的字节数 */
    uint32_t br;

    qh = *code;
    ql = *(++code);

    if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff)     /* 非 常用汉字 */
    {
        for (i = 0; i < csize; i++) *mat++ = 0x00;  /* 填充满格 */
        return;     /* 结束访问 */
    }

    if (ql < 0x7f) ql -= 0x40;
    else ql -= 0x41;

    qh -= 0x81;
    foffset = ((unsigned long)190 * qh + ql) * csize;   /* 得到字库中的字节偏移量 */

    /* 
     * 读取逻辑：
     * 1. 检查是否开启了加速模式（文件已打开）
     * 2. 如果开启了，直接 seek + read
     * 3. 如果没开启，临时打开文件读取（兼容普通显示）
     */
    if (s_is_font_open && s_current_size == size)
    {
        f_lseek(&s_font_file, foffset);
        f_read(&s_font_file, mat, csize, &br);
    }
    else
    {
        FIL temp_fil;
        char *path = NULL;
        
        switch (size)
        {
            case 12: path = "/SYSTEM/FONT/GBK12.FON"; break;
            case 16: path = "/SYSTEM/FONT/GBK16.FON"; break;
            case 24: path = "/SYSTEM/FONT/GBK24.FON"; break;
        }
        
        if (path && f_open(&temp_fil, path, FA_READ) == FR_OK)
        {
            f_lseek(&temp_fil, foffset);
            f_read(&temp_fil, mat, csize, &br);
            f_close(&temp_fil);
        }
        else
        {
            /* 读取失败，清空缓冲区 */
            for (i = 0; i < csize; i++) mat[i] = 0;
        }
    }
}

/**
 * @brief       显示一个指定大小的汉字
 * @param       x,y   : 汉字的坐标
 * @param       font  : 汉字GBK码
 * @param       size  : 字体大小
 * @param       mode  : 显示模式
 *   @note              0, 正常显示(不需要显示的点,用LCD背景色填充,即g_back_color)
 *   @note              1, 叠加显示(仅显示需要显示的点, 不需要显示的点, 不做处理)
 * @param       color : 字体颜色
 * @retval      无
 */
void text_show_font(uint16_t x, uint16_t y, uint8_t *font, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t temp, t, t1;
    uint16_t y0 = y;
    
    /* 
     * 修改：定义局部数组代替 malloc 
     * 32*32字体需要 128字节，定义144字节留有余量
     */
    uint8_t dzk[144]; 
    
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size);     /* 得到字体一个字符对应点阵集所占的字节数 */

    if (size != 12 && size != 16 && size != 24 && size != 32)
    {
        return;     /* 不支持的size */
    }

    /* 改为调用修改后的读取函数 */
    text_get_hz_mat(font, dzk, size);   /* 得到相应大小的点阵数据 */

    for (t = 0; t < csize; t++)
    {
        temp = dzk[t];                  /* 得到点阵数据 */

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
            {
                lcd_draw_point(x, y, color);        /* 画需要显示的点 */
            }
            else if (mode == 0)                     /* 如果非叠加模式, 不需要显示的点,用背景色填充 */
            {
                lcd_draw_point(x, y, g_back_color); /* 填充背景色 */
            }

            temp <<= 1;
            y++;

            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @brief       在指定位置开始显示一个字符串
 *   @note      该函数支持自动换行
 * @param       x,y   : 起始坐标
 * @param       width : 显示区域宽度
 * @param       height: 显示区域高度
 * @param       str   : 字符串
 * @param       size  : 字体大小
 * @param       mode  : 显示模式
 *   @note              0, 正常显示(不需要显示的点,用LCD背景色填充,即g_back_color)
 *   @note              1, 叠加显示(仅显示需要显示的点, 不需要显示的点, 不做处理)
 * @param       color : 字体颜色
 * @retval      无
 */
void text_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *str, uint8_t size, uint8_t mode, uint16_t color)
{
    uint16_t x0 = x;
    uint16_t y0 = y;
    uint8_t bHz = 0;                /* 字符或者中文 */
    uint8_t *pstr = (uint8_t *)str; /* 指向char*型字符串首地址 */

    while (*pstr != 0)              /* 数据未结束 */
    {
        if (!bHz)
        {
            if (*pstr > 0x80)       /* 中文 */
            {
                bHz = 1;            /* 标记是中文 */
            }
            else    /* 字符 */
            {
                if (x > (x0 + width - size / 2))    /* 换行 */
                {
                    y += size;
                    x = x0;
                }

                if (y > (y0 + height - size))break; /* 越界返回 */

                if (*pstr == 13)   /* 换行符号 */
                {
                    y += size;
                    x = x0;
                    pstr++;
                }
                else
                {
                    lcd_show_char(x, y, *pstr, size, mode, color);   /* 有效部分写入 */
                }

                pstr++;

                x += size / 2;  /* 英文字符宽度, 为中文汉字宽度的一半 */
            }
        }
        else    /* 中文 */
        {
            bHz = 0;    /* 有汉字库 */

            if (x > (x0 + width - size))        /* 换行 */
            {
                y += size;
                x = x0;
            }

            if (y > (y0 + height - size))break; /* 越界返回 */

            text_show_font(x, y, pstr, size, mode, color); /* 显示这个汉字,空心显示 */
            pstr += 2;
            x += size; /* 下一个汉字偏移 */
        }
    }
}

/**
 * @brief       在指定宽度的中间显示字符串
 *   @note      如果字符长度超过了len,则用text_show_string_middle显示
 * @param       x,y   : 起始坐标
 * @param       str   : 字符串
 * @param       size  : 字体大小
 * @param       width : 显示区域宽度
 * @param       color : 字体颜色
 * @retval      无
 */
void text_show_string_middle(uint16_t x, uint16_t y, char *str, uint8_t size, uint16_t width, uint16_t color)
{
    uint16_t strlenth = 0;
    strlenth = strlen((const char *)str);
    strlenth *= size / 2;

    if (strlenth > width) /* 超过了, 不能居中显示 */
    {
        text_show_string(x, y, lcddev.width, lcddev.height, str, size, 1, color);
    }
    else
    {
        strlenth = (width - strlenth) / 2;
        text_show_string(strlenth + x, y, lcddev.width, lcddev.height, str, size, 1, color);
    }
}




///**
// ****************************************************************************************************
// * @file        text.c
// * @author      正点原子团队(ALIENTEK)
// * @version     V1.0
// * @date        2021-11-06
// * @brief       汉字显示 代码
// *              提供text_show_font和text_show_string两个函数,用于显示汉字
// * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
// ****************************************************************************************************
// * @attention
// *
// * 实验平台:正点原子 STM32开发板
// * 在线视频:www.yuanzige.com
// * 技术论坛:www.openedv.com
// * 公司网址:www.alientek.com
// * 购买地址:openedv.taobao.com
// *
// * 修改说明
// * V1.0 20211106
// * 第一次发布
// *
// ****************************************************************************************************
// */

//#include "string.h"
//#include "./TEXT/text.h"
//#include "./BSP/LCD/lcd.h"
////#include "./MALLOC/malloc.h"
//#include "./SYSTEM/usart/usart.h"
//#include "./BSP/NORFLASH/norflash.h"


///**
// * @brief       获取汉字点阵数据
// * @param       code  : 当前汉字编码(GBK码)
// * @param       mat   : 当前汉字点阵数据存放地址
// * @param       size  : 字体大小
// *   @note      size大小的字体,其点阵数据大小为: (size / 8 + ((size % 8) ? 1 : 0)) * (size)  字节
// * @retval      无
// */
//static void text_get_hz_mat(unsigned char *code, unsigned char *mat, uint8_t size)
//{
//    unsigned char qh, ql;
//    unsigned char i;
//    unsigned long foffset;
//    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size); /* 得到字体一个字符对应点阵集所占的字节数 */
//    qh = *code;
//    ql = *(++code);

//    if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff)     /* 非 常用汉字 */
//    {
//        for (i = 0; i < csize; i++)
//        {
//            *mat++ = 0x00;  /* 填充满格 */
//        }

//        return;     /* 结束访问 */
//    }

//    if (ql < 0x7f)
//    {
//        ql -= 0x40; /* 注意! */
//    }
//    else
//    {
//        ql -= 0x41;
//    }

//    qh -= 0x81;
//    foffset = ((unsigned long)190 * qh + ql) * csize;   /* 得到字库中的字节偏移量 */

//    switch (size)
//    {
//        case 12:
//            norflash_read(mat, foffset + ftinfo.f12addr, csize);
//            break;

//        case 16:
//            norflash_read(mat, foffset + ftinfo.f16addr, csize);
//            break;

//        case 24:
//            norflash_read(mat, foffset + ftinfo.f24addr, csize);
//            break;
//    }
//}

///**
// * @brief       显示一个指定大小的汉字
// * @param       x,y   : 汉字的坐标
// * @param       font  : 汉字GBK码
// * @param       size  : 字体大小
// * @param       mode  : 显示模式
// *   @note              0, 正常显示(不需要显示的点,用LCD背景色填充,即g_back_color)
// *   @note              1, 叠加显示(仅显示需要显示的点, 不需要显示的点, 不做处理)
// * @param       color : 字体颜色
// * @retval      无
// */
//void text_show_font(uint16_t x, uint16_t y, uint8_t *font, uint8_t size, uint8_t mode, uint16_t color)
//{
//    uint8_t temp, t, t1;
//    uint16_t y0 = y;
//    uint8_t *dzk;
//    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size);     /* 得到字体一个字符对应点阵集所占的字节数 */

//    if (size != 12 && size != 16 && size != 24 && size != 32)
//    {
//        return;     /* 不支持的size */
//    }
//	uint8_t dzk[144]; 
//    uint8_t *p_dzk = dzk; // 指针指向局部数组

////    dzk = mymalloc(SRAMIN, size);       /* 申请内存 */

//    if (dzk == 0) return;               /* 内存不够了 */

//    text_get_hz_mat(font, dzk, size);   /* 得到相应大小的点阵数据 */

//    for (t = 0; t < csize; t++)
//    {
//        temp = dzk[t];                  /* 得到点阵数据 */

//        for (t1 = 0; t1 < 8; t1++)
//        {
//            if (temp & 0x80)
//            {
//                lcd_draw_point(x, y, color);        /* 画需要显示的点 */
//            }
//            else if (mode == 0)                     /* 如果非叠加模式, 不需要显示的点,用背景色填充 */
//            {
//                lcd_draw_point(x, y, g_back_color); /* 填充背景色 */
//            }

//            temp <<= 1;
//            y++;

//            if ((y - y0) == size)
//            {
//                y = y0;
//                x++;
//                break;
//            }
//        }
//    }

////    myfree(SRAMIN, dzk);    /* 释放内存 */
//}

///**
// * @brief       在指定位置开始显示一个字符串
// *   @note      该函数支持自动换行
// * @param       x,y   : 起始坐标
// * @param       width : 显示区域宽度
// * @param       height: 显示区域高度
// * @param       str   : 字符串
// * @param       size  : 字体大小
// * @param       mode  : 显示模式
// *   @note              0, 正常显示(不需要显示的点,用LCD背景色填充,即g_back_color)
// *   @note              1, 叠加显示(仅显示需要显示的点, 不需要显示的点, 不做处理)
// * @param       color : 字体颜色
// * @retval      无
// */
//void text_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *str, uint8_t size, uint8_t mode, uint16_t color)
//{
//    uint16_t x0 = x;
//    uint16_t y0 = y;
//    uint8_t bHz = 0;                /* 字符或者中文 */
//    uint8_t *pstr = (uint8_t *)str; /* 指向char*型字符串首地址 */

//    while (*pstr != 0)              /* 数据未结束 */
//    {
//        if (!bHz)
//        {
//            if (*pstr > 0x80)       /* 中文 */
//            {
//                bHz = 1;            /* 标记是中文 */
//            }
//            else    /* 字符 */
//            {
//                if (x > (x0 + width - size / 2))    /* 换行 */
//                {
//                    y += size;
//                    x = x0;
//                }

//                if (y > (y0 + height - size))break; /* 越界返回 */

//                if (*pstr == 13)   /* 换行符号 */
//                {
//                    y += size;
//                    x = x0;
//                    pstr++;
//                }
//                else
//                {
//                    lcd_show_char(x, y, *pstr, size, mode, color);   /* 有效部分写入 */
//                }

//                pstr++;

//                x += size / 2;  /* 英文字符宽度, 为中文汉字宽度的一半 */
//            }
//        }
//        else    /* 中文 */
//        {
//            bHz = 0;    /* 有汉字库 */

//            if (x > (x0 + width - size))        /* 换行 */
//            {
//                y += size;
//                x = x0;
//            }

//            if (y > (y0 + height - size))break; /* 越界返回 */

//            text_show_font(x, y, pstr, size, mode, color); /* 显示这个汉字,空心显示 */
//            pstr += 2;
//            x += size; /* 下一个汉字偏移 */
//        }
//    }
//}

///**
// * @brief       在指定宽度的中间显示字符串
// *   @note      如果字符长度超过了len,则用text_show_string_middle显示
// * @param       x,y   : 起始坐标
// * @param       str   : 字符串
// * @param       size  : 字体大小
// * @param       width : 显示区域宽度
// * @param       color : 字体颜色
// * @retval      无
// */
//void text_show_string_middle(uint16_t x, uint16_t y, char *str, uint8_t size, uint16_t width, uint16_t color)
//{
//    uint16_t strlenth = 0;
//    strlenth = strlen((const char *)str);
//    strlenth *= size / 2;

//    if (strlenth > width) /* 超过了, 不能居中显示 */
//    {
//        text_show_string(x, y, lcddev.width, lcddev.height, str, size, 1, color);
//    }
//    else
//    {
//        strlenth = (width - strlenth) / 2;
//        text_show_string(strlenth + x, y, lcddev.width, lcddev.height, str, size, 1, color);
//    }
//}













