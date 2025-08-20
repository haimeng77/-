/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       SPDIF��������Ƶ�� ʵ��
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
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
#include "./BSP/SPDIF/spdif.h"
#include "./BSP/SAI/sai.h"
#include "string.h"


#define     SPDIF_DBUF_SIZE     1024            /* ����SPDIF���ݽ��ջ������Ĵ�С,1K*4�ֽ� */

uint32_t spdif_audiobuf[2][SPDIF_DBUF_SIZE];    /* SPDIF��Ƶ���ݽ��ջ�����,��2��(˫����) */

/**
 * @brief       ��ʾ������
 * @param       samplerate:��Ƶ������(��λ:Hz)
 * @retval      ��
 */
void spdif_show_samplerate(uint32_t samplerate)
{
    uint8_t *buf;
    buf = mymalloc(SRAMIN, 100);                /* �����ڴ� */

    if (buf)                                    /* ����ɹ� */
    {
        if (samplerate)
        {
            sprintf((char *)buf, "%dHz", samplerate);   /* ��ӡ������ */
        }
        else
        {
            sprintf((char *)buf, "����ʶ��...");        /* ������Ϊ0,����ʾ����ʶ�� */
        }

        lcd_fill(30 + 56, 170, 230, 170 + 16, WHITE);
        text_show_string(30 + 56, 170, 200, 16, (char *)buf, 16, 0, RED);
    }

    myfree(SRAMIN, buf);                        /* �ͷ��ڴ� */
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
    memset((uint8_t *)&spdif_audiobuf[0], 0, SPDIF_DBUF_SIZE * 4);
    memset((uint8_t *)&spdif_audiobuf[1], 0, SPDIF_DBUF_SIZE * 4);
}


int main(void)
{
    uint8_t t;
    uint8_t key;
    uint8_t vol = 45;                       /* Ĭ������ */

    sys_cache_enable();                     /* ��L1-Cache */
    HAL_Init();                             /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(115200);                     /* ���ڳ�ʼ�� */
    usmart_init(200);                       /* ��ʼ��USMART */
    mpu_memory_protection();                /* ������ش洢���� */
    led_init();                             /* ��ʼ��LED */
    key_init();                             /* ��ʼ��KEY */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    norflash_init();                        /* ��ʼ��norflash */
    es8388_init();                          /* ES8388��ʼ�� */
    es8388_hpvol_set(25);                   /* ���ö������� */
    es8388_spkvol_set(10);                  /* ������������ */
    spdif_rx_init();                        /* SPDIF��ʼ�� */

    my_mem_init(SRAMIN);                    /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                    /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                    /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                  /* ��ʼ��ITCM�ڴ��(ITCM) */

    exfuns_init();                          /* Ϊfatfs��ر��������ڴ� */
    f_mount(fs[0], "0:", 1);                /* ����SD�� */

    while (fonts_init())                    /* ����ֿ� */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 200 + 30, 50 + 16, WHITE);
        delay_ms(200);
    }

    text_show_string(30, 50, 200, 16, "����ԭ��STM32������", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "SPDIF(������Ƶ)ʵ��", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "����ԭ��@ALIENTEK", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY_UP:VOL+  KEY1:VOL-", 16, 0, RED);

    text_show_string(30, 150, 200, 16, "����:", 16, 0, RED);
    text_show_string(30, 170, 200, 16, "������:", 16, 0, RED);
    lcd_show_num(30 + 40, 150, vol, 2, 16, BLUE);           /* ��ʾ���� */
    spdif_show_samplerate(0);                               /* ��ʾ������ */

    es8388_adda_cfg(1, 0);                                  /* ����DAC */
    es8388_input_cfg(0);                                    /* �ر�����ͨ�� */
    es8388_output_cfg(1, 0);                                /* ����DAC��� */
    spdif_rx_dma_init((uint32_t *)&spdif_audiobuf[0], (uint32_t *)&spdif_audiobuf[1], SPDIF_DBUF_SIZE, 2);
    spdif_rx_stop_callback = spdif_rx_stopplay_callback;    /* SPDIF ��������ʱ�Ļص����� */

    while (1)
    {
        key = key_scan(1);

        if (key == WKUP_PRES || key == KEY1_PRES)           /* �������� */
        {
            if (key == WKUP_PRES)
            {
                vol++;

                if (vol > 63)
                {
                    vol = 63;
                }
            }
            else
            {
                if (vol > 0)
                {
                    vol--;
                }
            }

            es8388_hpvol_set(vol);                                  /* ���ö����������� */
            es8388_spkvol_set(vol);                                 /* ���������������� */
            lcd_show_num(30 + 40, 150, vol, 2, 16, 0X80);           /* ��ʾ���� */
        }

        if (spdif_dev.consta == 0)                                  /* δ���� */
        {
            if (spdif_rx_waitsync())                                /* �ȴ�ͬ�� */
            {
                spdif_dev.samplerate = spdif_rx_getsamplerate();    /* ��ò����� */

                if (spdif_dev.saisync == 0)                         /* SAIʱ�Ӻ�SPDIFʱ�ӻ�δͬ��,���Ƚ���ͬ�� */
                {
                    if (spdif_dev.samplerate)                       /* ��������Ч,ͬ�� */
                    {
                        es8388_sai_cfg(0, 0);                       /* �����ֱ�׼,24λ���ݳ��� */
                        sai1_saia_init(SAI_MODEMASTER_TX, SAI_CLOCKSTROBING_RISINGEDGE, SAI_DATASIZE_24); /* ����SAI,������,16λ���� */
                        sai1_samplerate_set(spdif_dev.samplerate);  /* ���ò�����,���޸�plln��ֵ */
                        spdif_dev.saisync = 1;                      /* ���SAI��SPDIFʱ��ͬ����� */
                    }
                }
                else if (spdif_dev.samplerate)                      /* SAI��SPDIFʱ���Ѿ�ͬ�����Ҳ���������,SPDIF�������������� */
                {
                    spdif_play_start();                             /* ͬ����ɣ���SPDIF */
                    sai1_tx_dma_init((uint8_t *)&spdif_audiobuf[0], (uint8_t *)&spdif_audiobuf[1], SPDIF_DBUF_SIZE, 2); /* ����TX DMA,32λ */
                    sai1_tx_callback = sai_dma_tx_callback;
                    sai1_play_start();                              /* ����DMA */
                    spdif_show_samplerate(spdif_dev.samplerate);    /* ��ʾ������. */
                }

                else 
                {
                    spdif_play_stop();        /* �����ʴ���,ֹͣSPDIF���� */
                }
            }

            else 
            {
                LED0_TOGGLE();      /* LED0��˸ */
            }
        }

        delay_ms(20);
        t++;

        if (t > 10)
        {
            t = 0;
            LED0_TOGGLE();
        }
    }
}

