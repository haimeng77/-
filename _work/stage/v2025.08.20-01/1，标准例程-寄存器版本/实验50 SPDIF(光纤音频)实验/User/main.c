/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-25
 * @brief       SPDIF(������Ƶ) ʵ��
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
#include "./BSP/SPDIF/spdif.h"
#include "string.h"


#define     SPDIF_DBUF_SIZE     1024            /* ����SPDIF���ݽ��ջ������Ĵ�С,1K*4�ֽ� */

uint32_t g_spdif_audiobuf[2][SPDIF_DBUF_SIZE];  /* SPDIF��Ƶ���ݽ��ջ�����,��2��(˫����) */

/**
 * @brief       ��ʾ��Ƶ������
 * @param       samplerate  : ��Ƶ������(��λ:Hz)
 * @retval      ��
 */
void spdif_show_samplerate(uint32_t samplerate)
{
    uint8_t *buf;
    buf = mymalloc(SRAMIN, 100);    /* �����ڴ� */

    if (buf)                        /* ����ɹ� */
    {
        if (samplerate)
        {
            sprintf((char*)buf, "%dHz", samplerate); /* ��ӡ������ */
        }
        else
        {
            sprintf((char*)buf, "����ʶ��...");     /* ������Ϊ0,����ʾ����ʶ�� */
        }

        lcd_fill(30 + 56, 170, 230, 170 + 16, WHITE);
        text_show_string(30 + 56, 170, 200, 16, (char *)buf, 16, 0, BLUE);
    }
 
    myfree(SRAMIN, buf);       /* �ͷ��ڴ� */
}

/**
 * @brief       SAI DMA��������жϻص�����
 * @note        ����δ���κδ���.
 * @param       ��
 * @retval      ��
 */
void sai_dma_tx_callback(void)
{
}

/**
 * @brief       SPDIF RX����ʱ�Ļص�����
 * @param       ��
 * @retval      ��
 */
void spdif_rx_stopplay_callback(void)
{
    sai1_play_stop();
    SPDIFRX->IFCR |= 1 << 5;     /* ���ͬ����ɱ�־ */
    spdif_show_samplerate(0);
    
    memset((uint8_t *)&g_spdif_audiobuf[0], 0, SPDIF_DBUF_SIZE * 4);
    memset((uint8_t *)&g_spdif_audiobuf[1], 0, SPDIF_DBUF_SIZE * 4);
}

int main(void)
{
    uint8_t i;
    uint8_t volume = 25;
    uint8_t key = 0;
    
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
    spdif_rx_init();                        /* SPDIF��ʼ�� */

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

    text_show_string(30, 50, 200, 16, "����ԭ��STM32������", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "SPDIF(������Ƶ)ʵ��", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "����ԭ��@ALIENTEK", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY_UP:VOL+  KEY1:VOL-", 16, 0, RED);
    text_show_string(30, 150, 200, 16, "����:", 16, 0, RED);
    text_show_string(30, 170, 200, 16, "������:", 16, 0, RED);
    lcd_show_num(30 + 40, 150, volume, 2, 16, BLUE); /* ��ʾ���� */
    
    es8388_init();                          /* ES8388��ʼ�� */
    es8388_adda_cfg(1, 0);                  /* ����DAC�ر�ADC */
    es8388_output_cfg(1, 1);                /* DACѡ��ͨ����� */
    es8388_hpvol_set(volume);               /* ���ö������� */
    es8388_spkvol_set(volume);              /* ������������ */

    spdif_show_samplerate(0);               /* ��ʾ������ */
    spdif_rx_dma_init((uint32_t *)&g_spdif_audiobuf[0], (uint32_t *)&g_spdif_audiobuf[1], SPDIF_DBUF_SIZE, 2);
    spdif_rx_stop_callback = spdif_rx_stopplay_callback;    /* SPDIF ��������ʱ�Ļص����� */

    while (1)
    {
        key = key_scan(0);

        if (key == WKUP_PRES || key == KEY1_PRES)   /* �������� */
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

            es8388_hpvol_set(volume);           /* ���ö����������� */
            es8388_spkvol_set(volume);          /* ���������������� */
            lcd_show_num(30 + 40, 150, volume, 2, 16, BLUE);        /* ��ʾ���� */
        }

        if (spdif_dev.consta == 0)              /* δ���� */
        {
            if (spdif_rx_wait_sync())           /* �ȴ�ͬ�� */
            {
                spdif_dev.samplerate = spdif_rx_get_samplerate();   /* ��ò����� */
                
                if (spdif_dev.samplerate)                           /* ��������Ч, SPDIF�������������� */
                {
                    es8388_i2s_cfg(0, 0);                           /* �����ֱ�׼,24λ���ݳ��� */
                    sai1_saia_init(0, 1, 6);                        /* ����SAI,������,24λ���� */
                    sai1_samplerate_set(spdif_dev.samplerate);      /* ���ò����� */
                    
                    spdif_rx_start();                               /* ͬ����ɣ���SPDIF */
                    sai1_tx_dma_init((uint8_t *)&g_spdif_audiobuf[0], (uint8_t *)&g_spdif_audiobuf[1], SPDIF_DBUF_SIZE, 2); /* ����TX DMA,32λ */
                    sai1_tx_callback = sai_dma_tx_callback;
                    sai1_play_start();                              /* ����DMA */
                    spdif_show_samplerate(spdif_dev.samplerate);    /* ��ʾ������ */
                }
                else 
                {
                    spdif_rx_stop();                                /* �����ʴ���,ֹͣSPDIF���� */
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


















