/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ����IAP ʵ��
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
#include "./IAP/iap.h"


extern uint32_t g_usart_rx_cnt;

int main(void)
{
    uint8_t t;
    uint8_t key;
    uint32_t oldcount = 0;                      /* �ϵĴ��ڽ�������ֵ */
    uint32_t applenth = 0;                      /* ���յ���app���볤�� */
    uint8_t clearflag = 0;

    sys_cache_enable();                         /* ��L1-Cache */
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);         /* ����ʱ��, 400Mhz */
    delay_init(400);                            /* ��ʱ��ʼ�� */
    usart_init(115200);                         /* ���ڳ�ʼ�� */
    mpu_memory_protection();                    /* ������ش洢���� */
    led_init();                                 /* ��ʼ��LED */
    key_init();                                 /* ��ʼ��KEY */
    sdram_init();                               /* ��ʼ��SDRAM */
    lcd_init();                                 /* ��ʼ��LCD */

    lcd_show_string(30,  50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30,  70, 200, 16, 16, "IAP TEST", RED);
    lcd_show_string(30,  90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY_UP: Copy APP2FLASH!", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY0: Run SRAM APP", RED);
    lcd_show_string(30, 150, 200, 16, 16, "KEY1: Erase SRAM APP", RED);
    lcd_show_string(30, 170, 200, 16, 16, "KEY2: Run FLASH APP", RED);

    while (1)
    {
        if (g_usart_rx_cnt)
        {
            if (oldcount == g_usart_rx_cnt)     /* ��������,û���յ��κ�����,��Ϊ�������ݽ������ */
            {
                applenth = g_usart_rx_cnt;
                oldcount = 0;
                g_usart_rx_cnt = 0;
                printf("�û�����������!\r\n");
                printf("���볤��:%dBytes\r\n", applenth);
            }
            else
            {
                oldcount = g_usart_rx_cnt;
            }
        }

        t++;
        delay_ms(100);

        if (t == 3)
        {
            LED0_TOGGLE();
            t = 0;

            if (clearflag)
            {
                clearflag--;

                if (clearflag == 0)
                {
                    lcd_fill(30, 190, 240, 210 + 16, WHITE);               /* �����ʾ */
                }
            }
        }

        key = key_scan(0);

        if (key == WKUP_PRES)                  /* WKUP����,���¹̼���FLASH */
        {
            if (applenth)
            {
                printf("��ʼ���¹̼�...\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "Copying APP2FLASH...", BLUE);

                if (((*(volatile uint32_t *)(0x24001000 + 4)) & 0xFF000000) == 0x08000000)  /* �ж��Ƿ�Ϊ0X08XXXXXX */
                {
                    iap_write_appbin(FLASH_APP1_ADDR, g_usart_rx_buf, applenth);            /* ����FLASH���� */
                    lcd_show_string(30, 190, 200, 16, 16, "Copy APP Successed!!", BLUE);
                    printf("�̼��������!\r\n");
                }
                else
                {
                    lcd_show_string(30, 190, 200, 16, 16, "Illegal FLASH APP!  ", BLUE);
                    printf("��FLASHӦ�ó���!\r\n");
                }
            }
            else
            {
                printf("û�п��Ը��µĹ̼�!\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "No APP!", BLUE);
            }

            clearflag = 7;                      /* ��־��������ʾ,��������7*300ms�������ʾ */
        }

        if (key == KEY1_PRES)                   /* KEY1�������� */
        {
            if (applenth)
            {
                printf("�̼�������!\r\n");    
                lcd_show_string(30, 190, 200, 16, 16, "APP Erase Successed!", BLUE);
                applenth = 0;
            }
            else 
            {
                printf("û�п�������Ĺ̼�!\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "No APP!", BLUE);
            }

            clearflag = 7;                      /* ��־��������ʾ,��������7*300ms�������ʾ */
        }

        if (key == KEY2_PRES)                   /* KEY2���� */
        {
            printf("��ʼִ��FLASH�û�����!!\r\n");
            if (((*(volatile uint32_t *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000)   /* �ж��Ƿ�Ϊ0X08XXXXXX.*/
            {
                iap_load_app(FLASH_APP1_ADDR);                                                /* ִ��FLASH APP���� */
            }
            else 
            {
                printf("��FLASHӦ�ó���,�޷�ִ��!\r\n");
                lcd_show_string(30, 210, 200, 16, 16, "Illegal FLASH APP!", BLUE);
            }

            clearflag = 7;                                                                    /* ��־��������ʾ,��������7*300ms�������ʾ */
        }
        
        if (key == KEY0_PRES)                                                                 /* KEY0���� */
        {
            printf("��ʼִ��SRAM�û�����!!\r\n");
            delay_ms(10);

            if (((*(volatile uint32_t *)(0x24001000 + 4)) & 0xFF000000) == 0x24000000)        /* �ж��Ƿ�Ϊ0X24XXXXXX */
            {
                iap_load_app(0x24001000);                                                     /* SRAM��ַ */
            }
            else
            {
                printf("��SRAMӦ�ó���,�޷�ִ��!\r\n");
                lcd_show_string(30, 190, 200, 16, 16, "Illegal SRAM APP!", BLUE);
            }

            clearflag = 7;                      /* ��־��������ʾ,��������7*300ms�������ʾ */
        }
    }
}


