/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       DMA ʵ��
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
#include "./BSP/DMA/dma.h"


#define SEND_BUF_SIZE       (sizeof(TEXT_TO_SEND) + 2) * 200                    /* �������ݳ���, ����sizeof(TEXT_TO_SEND) + 2��200��. */

const uint8_t TEXT_TO_SEND[] = {"����ԭ�� STM32 DMA ����ʵ��"};                  /* Ҫѭ�����͵��ַ��� */

uint8_t g_sendbuf[SEND_BUF_SIZE];                                               /* �������ݻ����� */


int main(void)
{
    uint8_t  key = 0;
    uint16_t i, k;
    uint16_t len;
    uint8_t  mask = 0;
    float pro = 0;                          /* ����:0~100 */
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                /* ��ʼ��USART */
    led_init();                             /* ��ʼ��LED */ 
    mpu_memory_protection();                /* ������ش洢���� */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    key_init();                             /* ��ʼ������ */
   
    dma_usart_tx_config(DMA2_Stream7, 42, (uint32_t)&USART1->TDR, (uint32_t)g_sendbuf);


    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "DMA TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Start", RED);

    len = sizeof(TEXT_TO_SEND);
    k = 0;
    
    for (i = 0; i < SEND_BUF_SIZE; i++) /* ���ASCII�ַ������� */
    {
        if (k >= len)   /* �뻻�з� */
        {
            if (mask)
            {
                g_sendbuf[i] = 0x0a;
                k = 0;
            }
            else
            {
                g_sendbuf[i] = 0x0d;
                mask++;
            }
        }
        else     /* ����TEXT_TO_SEND��� */
        {
            mask = 0;
            g_sendbuf[i] = TEXT_TO_SEND[k];
            k++;
        }
    }
 
    i = 0;

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)   /* KEY0���� */
        {
            printf("\r\nDMA DATA:\r\n");
            lcd_show_string(30, 130, 200, 16, 16, "Start Transimit....", BLUE);
            lcd_show_string(30, 150, 200, 16, 16, "   %", BLUE);    /* ��ʾ�ٷֺ� */
            USART1->CR3 = 1 << 7;       /* ʹ�ܴ���1��DMA���� */
            
            dma_enable(DMA2_Stream7, SEND_BUF_SIZE); /* ��ʼһ��DMA���䣡 */

            /* �ȴ�DMA������ɣ���ʱ������������һЩ���飬������  
             * ʵ��Ӧ���У����������ڼ䣬����ִ����������� 
             */
            while (1)
            {
                if (DMA2->HISR & (1 << 27)) /* �ȴ�DMA2_Steam7������� */
                {
                    DMA2->HIFCR |= 1 << 27; /* ���DMA2_Steam7������ɱ�־ */
                    break;
                }

                pro = DMA2_Stream7->NDTR;   /* �õ���ǰ��ʣ����ٸ����� */
                len = SEND_BUF_SIZE;        /* �ܳ��� */
                pro = 1 - (pro / len);      /* �õ��ٷֱ� */
                pro *= 100;                 /* ����100�� */
                lcd_show_num(30, 150, pro, 3, 16, BLUE);
            } 
            lcd_show_num(30, 150, 100, 3, 16, BLUE);    /* ��ʾ100% */
            lcd_show_string(30, 130, 200, 16, 16, "Transimit Finished!", BLUE); /* ��ʾ������� */
        }

        i++;
        delay_ms(10);

        if (i == 20)
        {
            LED0_TOGGLE();  /* LED0��˸,��ʾϵͳ�������� */
            i = 0;
        }
    }
}











