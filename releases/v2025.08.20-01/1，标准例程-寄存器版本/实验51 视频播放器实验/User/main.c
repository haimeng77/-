/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-25
 * @brief       ��Ƶ������ ʵ��
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
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
#include "./APP/videoplayer.h"
#include "./BSP/TIMER/btim.h"


int main(void)
{
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                /* ��ʼ��USART */
    usmart_dev.init(100);                   /* ��ʼ��USMART */
    led_init();                             /* ��ʼ��LED */
    mpu_memory_protection();                /* ������ش洢���� */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    norflash_init();                        /* ��ʼ��W25Q256 */
    key_init();                             /* ��ʼ������ */
    btim_timx_int_init(10000 - 1, 20000 - 1);   /* 10Khz����,1�����ж�һ�� */

    my_mem_init(SRAMIN);                    /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                    /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                    /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                  /* ��ʼ��ITCM�ڴ��(ITCM) */

    exfuns_init();                          /* Ϊfatfs��ر��������ڴ� */

    f_mount(fs[0], "0:", 1);                /* ����SD�� */
    f_mount(fs[1], "1:", 1);                /* ����SPI FLASH */
    f_mount(fs[2], "2:", 1);                /* ����NAND FLASH */

     while (fonts_init())                   /* ����ֿ� */
    {
        lcd_show_string(30, 90, 200, 16, 16, "Font error!", RED);
        delay_ms(200);
        lcd_fill(20, 90, 200 + 20, 90 + 16, WHITE);
        delay_ms(200);
    }

    es8388_init();                              /* ES8388��ʼ�� */
    es8388_adda_cfg(1, 0);                      /* ����DAC�ر�ADC */
    es8388_input_cfg(0);
    es8388_output_cfg(1, 1);                    /* DACѡ��ͨ����� */
    es8388_hpvol_set(25);                       /* ���ö������� */
    es8388_spkvol_set(25);                      /* ������������ */

    text_show_string(30, 30, 200, 16, "����ԭ��STM32������", 16, 0, RED);
    text_show_string(30, 50, 200, 16, "��Ƶ������ʵ��", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "����ԭ��@ALIENTEK", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "KEY0:NEXT   KEY2:PREV", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY_UP:FF   KEY1��REW", 16, 0, RED);

    delay_ms(1200);

    while (1)
    {
        video_play();
    }
}













