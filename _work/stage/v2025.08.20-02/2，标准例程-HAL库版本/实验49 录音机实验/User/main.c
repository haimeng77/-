/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       录音机 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./TEXT/text.h"
#include "./BSP/LCD/ltdc.h"
#include "./MALLOC/malloc.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/IIC/myiic.h"
#include "./BSP/ES8388/es8388.h"
#include "./APP/recorder.h"


int main(void)
{
    sys_cache_enable();                        /* 打开L1-Cache */
    HAL_Init();                                /* 初始化HAL库 */
    sys_stm32_clock_init(160, 5, 2, 4);        /* 设置时钟, 400Mhz */
    delay_init(400);                           /* 延时初始化 */
    usart_init(115200);                        /* 串口初始化 */
    usmart_init(200);                          /* 初始化USMART */
    mpu_memory_protection();                   /* 保护相关存储区域 */
    led_init();                                /* 初始化LED */
    key_init();                                /* 初始化KEY */
    sdram_init();                              /* 初始化SDRAM */
    lcd_init();                                /* 初始化LCD */
    norflash_init();                           /* 初始化norflash */

    es8388_init();                             /* ES8388初始化 */
    es8388_hpvol_set(25);                      /* 设置耳机音量 */
    es8388_spkvol_set(25);                     /* 设置喇叭音量 */

    my_mem_init(SRAMIN);                       /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                       /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAMDTCM);                     /* 初始化DTCM内存池(DTCM) */
       
    exfuns_init();                             /* 为fatfs相关变量申请内存 */
    f_mount(fs[0], "0:", 1);                   /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                   /* 挂载SPI FLASH */
    f_mount(fs[2], "2:", 1);                   /* 挂载NAND FLASH */
    
    while (sd_init())                          /* 检测SD卡 */
    {
        lcd_show_string(30, 50, 200, 16, 16, "SD Card Failed!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 200 + 30, 50 + 16, WHITE);
        delay_ms(200);
    }

    while (fonts_init())                       /* 检查字库 */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 200 + 30, 50 + 16, WHITE);
        delay_ms(200);
    }

    text_show_string(30, 50, 200, 16, "正点原子STM32开发板", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "WAV 录音机 实验", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "正点原子@ALIENTEK", 16, 0, RED);

    while (1)
    {
        wav_recorder();                        /* 录音 */
    }
}


