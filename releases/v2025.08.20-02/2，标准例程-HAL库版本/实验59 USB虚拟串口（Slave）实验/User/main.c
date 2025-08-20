/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       USB���⴮��(SLAVE) ʵ��
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
#include "./BSP/IIC/myiic.h"
#include "./MALLOC/malloc.h"
#include "./TEXT/text.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_conf.h" 
#include "usbd_cdc_interface.h"


USBD_HandleTypeDef USBD_Device;                   /* USB Device����ṹ�� */

extern volatile uint8_t g_device_state;           /* USB���� ��� */

int main(void)
{
    uint16_t len;
    uint16_t times = 0;
    uint8_t usbstatus = 0;

    sys_cache_enable();                          /* ��L1-Cache */
    mpu_memory_protection();                     /* ������ش洢���� */
    HAL_Init();                                  /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);          /* ����ʱ��, 400Mhz */
    delay_init(400);                             /* ��ʱ��ʼ�� */
    usart_init(115200);                          /* ���ڳ�ʼ�� */
    led_init();                                  /* ��ʼ��LED */
    sdram_init();                                /* ��ʼ��SDRAM */
    lcd_init();                                  /* ��ʼ��LCD */

    my_mem_init(SRAMIN);                         /* ��ʼ���ڲ��ڴ��(AXI) */
    my_mem_init(SRAMEX);                         /* ��ʼ���ⲿ�ڴ��(SDRAM) */
    my_mem_init(SRAM12);                         /* ��ʼ��SRAM12�ڴ��(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                          /* ��ʼ��SRAM4�ڴ��(SRAM4) */
    my_mem_init(SRAMDTCM);                       /* ��ʼ��DTCM�ڴ��(DTCM) */
    my_mem_init(SRAMITCM);                       /* ��ʼ��ITCM�ڴ��(ITCM) */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "USB Virtual USART TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "USB Connecting...", RED);              /* ��ʾUSB��ʼ���� */

    USBD_Init(&USBD_Device, &VCP_Desc, USE_USB_FS);                               /* ��ʼ��USB */
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);                             /* ����� */
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);                     /* ΪMSC����ӻص����� */
    USBD_Start(&USBD_Device);                                                     /* ����USB */
    delay_ms(1800);

    while (1)
    {
        delay_ms(1);
        if (usbstatus != g_device_state)         /* USB����״̬�����˸ı� */
        {
            usbstatus = g_device_state;          /* ��¼�µ�״̬ */

            if (usbstatus == 1)
            {
                lcd_show_string(30, 110, 200, 16, 16, "USB Connected    ", BLUE); /* ��ʾUSB���ӳɹ� */
                LED1(0);                                                          /* �̵��� */
            }
            else
            {
                lcd_show_string(30, 110, 200, 16, 16, "USB disConnected ", BLUE); /* ��ʾUSB�Ͽ� */
                LED1(1);                                                          /* �̵��� */
            }
        }

        if (g_usb_usart_rx_sta & 0x8000)
        {
            len = g_usb_usart_rx_sta & 0x3FFF;                                    /* �õ��˴ν��յ������ݳ��� */
            usb_printf("\r\n�����͵���Ϣ����Ϊ:%d\r\n\r\n", len);
            cdc_vcp_data_tx(g_usb_usart_rx_buffer, len);
            usb_printf("\r\n\r\n");                                               /* ���뻻�� */
            g_usb_usart_rx_sta = 0;
        }
        else
        {
            times++;

            if (times % 5000 == 0)
            {
                usb_printf("\r\nSTM32������USB���⴮��ʵ��\r\n");
                usb_printf("����ԭ��@ALIENTEK\r\n\r\n");
            }

            if (times % 200 == 0)
            {
                usb_printf("����������,�Իس�������\r\n");
            }

            if (times % 30 == 0)
            {
                LED0_TOGGLE();       /* ��˸LED,��ʾϵͳ�������� */
            }
            delay_ms(10);
        }
    }
}

