/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       USB����(SLAVE) ʵ��
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
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/ltdc.h"
#include "./BSP/IIC/myiic.h"
#include "./MALLOC/malloc.h"
#include "./BSP/ES8388/es8388.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./BSP/SAI/sai.h"
#include "./USMART/usmart.h"
#include "usbd_core.h"
#include "usbd_conf.h"
#include "usbd_desc.h"
#include "usbd_audio.h"
#include "usbd_audio_if.h"


USBD_HandleTypeDef USBD_Device;
extern PCD_HandleTypeDef hpcd;
extern uint8_t g_device_state;          /* USB���� ��� */
extern uint8_t g_volume;                /* ����(��ͨ����������) */

int main(void)
{
    uint8_t key;
    uint8_t t = 0;
    uint8_t device_sta = 0xFF;

    sys_cache_enable();                       /* ��L1-Cache */
    mpu_memory_protection();                  /* ������ش洢���� */
    HAL_Init();                               /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);       /* ����ʱ��, 400Mhz */
    delay_init(400);                          /* ��ʱ��ʼ�� */
    usart_init(115200);                       /* ���ڳ�ʼ�� */
    usmart_init(200);                         /* ��ʼ��USMART */
    led_init();                               /* ��ʼ��LED */
    key_init();                               /* ��ʼ��KEY */
    sdram_init();                             /* ��ʼ��SDRAM */
    lcd_init();                               /* ��ʼ��LCD */
    pcf8574_init();                           /* ��ʼ��PCF8574 */

    my_mem_init(SRAMIN);                      /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                      /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAMDTCM);                    /* ��ʼ��DTCM�ڴ��(DTCM) */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "USB Sound Card TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY2:Vol-  KEY0:Vol+", RED);

    lcd_show_string(30, 140, 200, 16, 16, "VOLUME:", BLUE);                 /* ��ʾ���� */
    lcd_show_xnum(30 + 56, 140, AUDIO_DEFAULT_VOLUME, 3, 16, 0x80, BLUE);   /* ��ʾ����ֵ */
    lcd_show_string(30, 180, 200, 16, 16, "USB Connecting...", RED);        /* ��ʾ���ڽ������� */

    es8388_init();                                                  /* ES8388��ʼ�� */
    es8388_adda_cfg(1, 0);                                          /* ����DAC�ر�ADC */
    es8388_output_cfg(1, 1);                                        /* DACѡ��ͨ����� */
    es8388_hpvol_set(15);                                           /* ���ö������� */
    es8388_spkvol_set(15);                                          /* ������������ */
    
    USBD_Init(&USBD_Device, &AUDIO_Desc, 0);                        /* ��ʼ��USB */
    USBD_RegisterClass(&USBD_Device, USBD_AUDIO_CLASS);             /* ����� */
    USBD_AUDIO_RegisterInterface(&USBD_Device, &USBD_AUDIO_fops);   /* ΪMSC����ӻص����� */
    USBD_Start(&USBD_Device);                                       /* ����USB */
    delay_ms(1800);

    while (1)
    {
        key = key_scan(1);              /* ֧������ */
        if (key)
        {
            if (key == KEY0_PRES)       /* KEY0����,�������� */
            {
                g_volume++;

                if (g_volume > 100)     /* ��Χ�޶� */
                {
                    g_volume = 100;
                }
            }
            else if (key == KEY2_PRES)  /* KEY2����,�������� */
            {
                if (g_volume)           /* ��Χ�޶� */
                {
                    g_volume--;
                }
            }

            es8388_hpvol_set(g_volume * 0.3);
            es8388_spkvol_set(g_volume * 0.3);
            lcd_show_xnum(30 + 56, 140, g_volume, 3, 16, 0x80, BLUE);   /* ��ʾ����ֵ */
            delay_ms(20);
        }

        if (device_sta != g_device_state)                               /* ״̬�ı��� */
        {
            if (g_device_state == 1)
            {
                lcd_show_string(30, 180, 200, 16, 16, "USB Connected    ", RED);    /* ��ʾUSB�����Ѿ����� */
                LED1(0);    /* LED1�� */
            }
            else
            {
                lcd_show_string(30, 180, 200, 16, 16, "USB DisConnected ", RED);    /* ��ʾUSB����ʧ�� */
                LED1(1);    /* LED1�� */
            }

            device_sta = g_device_state;
        }

        delay_ms(20);
        t++;

        if (t > 10)
        {
            t = 0;
            LED0_TOGGLE();  /* ��ʾϵͳ������ */
        }
    }
}

