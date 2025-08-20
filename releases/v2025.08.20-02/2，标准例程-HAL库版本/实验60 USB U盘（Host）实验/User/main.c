/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       USB U�̣�HOST�� ʵ��
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
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD/ltdc.h"
#include "./BSP/NAND/ftl.h"
#include "./BSP/NAND/nand.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/IIC/myiic.h"
#include "./PICTURE/piclib.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./MALLOC/malloc.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./BSP/PCF8574/pcf8574.h"
#include "./TEXT/text.h"
#include "./USMART/usmart.h"
#include "usbh_core.h"
#include "usbh_msc.h"



USBH_HandleTypeDef g_hUSBHost;              /* USB Host����ṹ�� */

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    uint32_t total, free;
    uint8_t res = 0;
    printf("id:%d\r\n", id);

    switch (id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            f_mount(0, "3:", 1);                                    /* ж��U�� */
            text_show_string(30, 120, 200, 16, "�豸������...", 16, 0, RED);
            LED1(1);
            lcd_fill(30, 160, 239, 220, WHITE);
            break;

        case HOST_USER_CLASS_ACTIVE:
            text_show_string(30, 120, 200, 16, "�豸���ӳɹ�!", 16, 0, RED);
            f_mount(fs[3], "3:", 1);                                /* ���¹���U�� */
            LED1(0);
            res = exfuns_get_free((uint8_t *)"3:", &total, &free);
            if (res == 0)
            {
                lcd_show_string(30, 160, 200, 16, 16, "FATFS OK!", BLUE);
                lcd_show_string(30, 180, 200, 16, 16, "U Disk Total Size:     MB", BLUE);
                lcd_show_string(30, 200, 200, 16, 16, "U Disk  Free Size:     MB", BLUE);
                lcd_show_num(174, 180, total >> 10, 5, 16, BLUE);   /* ��ʾU�������� MB */
                lcd_show_num(174, 200, free >> 10, 5, 16, BLUE);
            }
            else
            {
                printf("U�̴洢�ռ��ȡʧ��\r\n");
            }
            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }
}

int main(void)
{
    uint8_t t = 0;

    sys_cache_enable();                         /* ��L1-Cache */
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);         /* ����ʱ��, 400Mhz */
    delay_init(400);                            /* ��ʱ��ʼ�� */
    usart_init(115200);                         /* ���ڳ�ʼ�� */
    usmart_init(200);                           /* ��ʼ��USMART */
    mpu_memory_protection();                    /* ������ش洢���� */
    led_init();                                 /* ��ʼ��LED */
    sdram_init();                               /* ��ʼ��SDRAM */
    lcd_init();                                 /* ��ʼ��LCD */
    norflash_init();                            /* ��ʼ��norflash */
    piclib_init();                              /* ��ʼ����ͼ */
    pcf8574_init();                             /* ��ʼ��PCF8574 */

    my_mem_init(SRAMIN);                        /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                        /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                        /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                         /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                      /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                      /* ��ʼ��ITCM�ڴ��(ITCM) */

    exfuns_init();                              /* Ϊfatfs��ر��������ڴ� */
    f_mount(fs[0], "0:", 1);                    /* ����SD�� */
    f_mount(fs[1], "1:", 1);                    /* ����FLASH */
    f_mount(fs[2], "2:", 1);                    /* ����NAND FLASH */

    while (fonts_init())                        /* ����ֿ� */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font Error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 240, 66, WHITE);       /* �����ʾ */
        delay_ms(200);
    }
    
    text_show_string(30, 50, 200, 16, "STM32", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "USB U�� ʵ��", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "ATOM@ALIENTEK", 16, 0, RED);
    text_show_string(30, 120, 200, 16, "�豸������...", 16, 0, RED);

    USBH_Init(&g_hUSBHost, USBH_UserProcess, 0);
    USBH_RegisterClass(&g_hUSBHost, USBH_MSC_CLASS);
    USBH_Start(&g_hUSBHost);
    HAL_PWREx_EnableUSBVoltageDetector();

    while (1)
    {
        USBH_Process(&g_hUSBHost);
        delay_ms(10);
        t++;

        if (t == 50)
        {
            t = 0;
            LED0_TOGGLE();
        }
    }
}

