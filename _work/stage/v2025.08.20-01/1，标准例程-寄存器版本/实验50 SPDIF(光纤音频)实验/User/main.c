/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-25
 * @brief       SPDIF(光纤音频) 实验
 * @license     Copyright (c) 2022-2032, 广州市星翼电子科技有限公司
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
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./MALLOC/malloc.h"
#include "./BSP/NAND/ftl.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h" 
#include "./TEXT/text.h"
#include "./BSP/SAI/sai.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/SPDIF/spdif.h"
#include "string.h"


#define     SPDIF_DBUF_SIZE     1024            /* 定义SPDIF数据接收缓冲区的大小,1K*4字节 */

uint32_t g_spdif_audiobuf[2][SPDIF_DBUF_SIZE];  /* SPDIF音频数据接收缓冲区,共2个(双缓冲) */

/**
 * @brief       显示音频采样率
 * @param       samplerate  : 音频采样率(单位:Hz)
 * @retval      无
 */
void spdif_show_samplerate(uint32_t samplerate)
{
    uint8_t *buf;
    buf = mymalloc(SRAMIN, 100);    /* 申请内存 */

    if (buf)                        /* 申请成功 */
    {
        if (samplerate)
        {
            sprintf((char*)buf, "%dHz", samplerate); /* 打印采样率 */
        }
        else
        {
            sprintf((char*)buf, "正在识别...");     /* 采样率为0,则提示正在识别 */
        }

        lcd_fill(30 + 56, 170, 230, 170 + 16, WHITE);
        text_show_string(30 + 56, 170, 200, 16, (char *)buf, 16, 0, BLUE);
    }
 
    myfree(SRAMIN, buf);       /* 释放内存 */
}

/**
 * @brief       SAI DMA发送完成中断回调函数
 * @note        这里未做任何处理.
 * @param       无
 * @retval      无
 */
void sai_dma_tx_callback(void)
{
}

/**
 * @brief       SPDIF RX结束时的回调函数
 * @param       无
 * @retval      无
 */
void spdif_rx_stopplay_callback(void)
{
    sai1_play_stop();
    SPDIFRX->IFCR |= 1 << 5;     /* 清除同步完成标志 */
    spdif_show_samplerate(0);
    
    memset((uint8_t *)&g_spdif_audiobuf[0], 0, SPDIF_DBUF_SIZE * 4);
    memset((uint8_t *)&g_spdif_audiobuf[1], 0, SPDIF_DBUF_SIZE * 4);
}

int main(void)
{
    uint8_t i;
    uint8_t volume = 25;
    uint8_t key = 0;
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* 设置时钟, 400Mhz */
    delay_init(400);                        /* 延时初始化 */
    usart_init(100, 115200);                /* 初始化USART */
    usmart_dev.init(100);                   /* 初始化USMART */
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    norflash_init();                        /* 初始化W25Q256 */
    key_init();                             /* 初始化按键 */
    spdif_rx_init();                        /* SPDIF初始化 */

    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */

    exfuns_init();                          /* 为fatfs相关变量申请内存 */

    f_mount(fs[0], "0:", 1);                /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                /* 挂载SPI FLASH */
    f_mount(fs[2], "2:", 1);                /* 挂载NAND FLASH */

     while (fonts_init())                   /* 检查字库 */
    {
        lcd_show_string(30, 90, 200, 16, 16, "Font error!", RED);
        delay_ms(200);
        lcd_fill(20, 90, 200 + 20, 90 + 16, WHITE);
        delay_ms(200);
    }

    text_show_string(30, 50, 200, 16, "正点原子STM32开发板", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "SPDIF(光纤音频)实验", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "正点原子@ALIENTEK", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY_UP:VOL+  KEY1:VOL-", 16, 0, RED);
    text_show_string(30, 150, 200, 16, "音量:", 16, 0, RED);
    text_show_string(30, 170, 200, 16, "采样率:", 16, 0, RED);
    lcd_show_num(30 + 40, 150, volume, 2, 16, BLUE); /* 显示音量 */
    
    es8388_init();                          /* ES8388初始化 */
    es8388_adda_cfg(1, 0);                  /* 开启DAC关闭ADC */
    es8388_output_cfg(1, 1);                /* DAC选择通道输出 */
    es8388_hpvol_set(volume);               /* 设置耳机音量 */
    es8388_spkvol_set(volume);              /* 设置喇叭音量 */

    spdif_show_samplerate(0);               /* 显示采样率 */
    spdif_rx_dma_init((uint32_t *)&g_spdif_audiobuf[0], (uint32_t *)&g_spdif_audiobuf[1], SPDIF_DBUF_SIZE, 2);
    spdif_rx_stop_callback = spdif_rx_stopplay_callback;    /* SPDIF 结束播放时的回调函数 */

    while (1)
    {
        key = key_scan(0);

        if (key == WKUP_PRES || key == KEY1_PRES)   /* 音量控制 */
        {
            if (key == WKUP_PRES)
            {
                volume++;
                if (volume > 33)
                {
                    volume = 33;
                }
            }
            else
            {
                if (volume > 0)
                {
                    volume--;
                }
            }

            es8388_hpvol_set(volume);           /* 设置耳机音量设置 */
            es8388_spkvol_set(volume);          /* 设置喇叭音量设置 */
            lcd_show_num(30 + 40, 150, volume, 2, 16, BLUE);        /* 显示音量 */
        }

        if (spdif_dev.consta == 0)              /* 未连接 */
        {
            if (spdif_rx_wait_sync())           /* 等待同步 */
            {
                spdif_dev.samplerate = spdif_rx_get_samplerate();   /* 获得采样率 */
                
                if (spdif_dev.samplerate)                           /* 采样率有效, SPDIF可以正常工作了 */
                {
                    es8388_i2s_cfg(0, 0);                           /* 飞利浦标准,24位数据长度 */
                    sai1_saia_init(0, 1, 6);                        /* 设置SAI,主发送,24位数据 */
                    sai1_samplerate_set(spdif_dev.samplerate);      /* 设置采样率 */
                    
                    spdif_rx_start();                               /* 同步完成，打开SPDIF */
                    sai1_tx_dma_init((uint8_t *)&g_spdif_audiobuf[0], (uint8_t *)&g_spdif_audiobuf[1], SPDIF_DBUF_SIZE, 2); /* 配置TX DMA,32位 */
                    sai1_tx_callback = sai_dma_tx_callback;
                    sai1_play_start();                              /* 开启DMA */
                    spdif_show_samplerate(spdif_dev.samplerate);    /* 显示采样率 */
                }
                else 
                {
                    spdif_rx_stop();                                /* 采样率错误,停止SPDIF播放 */
                }                                                                                                                                                                
            }else
            {
                LED0_TOGGLE();
            }
        }

        delay_ms(10);
        i++;

        if (i > 20)
        {
            i = 0;
            LED0_TOGGLE();
        }
    }
}


















