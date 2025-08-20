/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       FDCANͨ�� ʵ��
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
#include "./BSP/FDCAN/fdcan.h"


int main(void)
{
    uint8_t key;
    uint8_t i = 0, t = 0;
    uint8_t cnt = 0;
    uint8_t canbuf[8];
    uint8_t rxlen = 0;
    uint8_t res;
    uint8_t mode = 1;                       /* CAN����ģʽ: 0,��ͨģʽ; 1,����ģʽ */

    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                /* ��ʼ��USART */
    usmart_dev.init(100);                   /* USMART��ʼ�� */
    led_init();                             /* ��ʼ��LED */
    mpu_memory_protection();                /* ������ش洢���� */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    key_init();                             /* ��ʼ������ */
    fdcan_init(10, 8, 31, 8, FDCAN_MODE_INTERNAL_LOOPBACK);     /* �ػ����� */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "FDCAN TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 130, 200, 16, 16, "LoopBack Mode", RED);
    lcd_show_string(30, 150, 200, 16, 16, "KEY0:Send WK_UP:Mode", RED);    /* ��ʾ��ʾ��Ϣ */

    lcd_show_string(30, 170, 200, 16, 16, "Count:", BLUE);
    lcd_show_string(30, 190, 200, 16, 16, "Send Data:", BLUE);
    lcd_show_string(30, 250, 200, 16, 16, "Receive Data:", BLUE);

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)              /* KEY0����,����һ������ */
        {
            for (i = 0; i < 8; i++)
            {
                canbuf[i] = cnt + i;       /* ��䷢�ͻ����� */

                if (i < 4)
                {
                    lcd_show_xnum(30 + i * 32, 210, canbuf[i], 3, 16, 0X80, BLUE);         /* ��ʾ���� */
                }
                else 
                {
                    lcd_show_xnum(30 + (i - 4) * 32, 230, canbuf[i], 3, 16, 0X80, BLUE);   /* ��ʾ���� */
                }
            }

            res = fdcan_send_msg(canbuf, FDCAN_DLC_BYTES_8);                           /* ����8���ֽ�  */

            if (res)
            {
                lcd_show_string(30 + 80, 190, 200, 16, 16, "Failed", BLUE);            /* ��ʾ����ʧ�� */
            }
            else 
            {
                lcd_show_string(30 + 80, 190, 200, 16, 16, "OK    ", BLUE);            /* ��ʾ���ͳɹ� */
            }
        }
        else if (key == WKUP_PRES)                  /* WK_UP���£��ı�CAN�Ĺ���ģʽ */
        {
            mode = !mode;
            if (mode == 0)  
            {
                fdcan_init(10, 8, 31, 8, FDCAN_MODE_NORMAL);            /* ����ģʽ,������500Kbps */
            }
            else if (mode == 1) 
            {
                fdcan_init(10, 8, 31, 8, FDCAN_MODE_INTERNAL_LOOPBACK);  /* �ػ�ģʽ,������500Kbps */
            }
            
            if (mode == 0)                 /* ��ͨģʽ����Ҫ2�������� */
            {
                lcd_show_string(30, 130, 200, 16, 16, "Nnormal Mode ", BLUE);
            }
            else                          /* �ػ�ģʽ,һ��������Ϳ��Բ�����. */
            {
                lcd_show_string(30, 130, 200, 16, 16, "LoopBack Mode", BLUE);
            }
        }

        rxlen = fdcan_receive_msg(canbuf);

        if (rxlen)                                    /* ���յ������� */
        {
            lcd_fill(30, 270, 160, 310, WHITE);       /* ���֮ǰ����ʾ */
            for (i = 0; i < rxlen; i++)
            {
                if (i < 4)
                {
                    lcd_show_xnum(30 + i * 32, 270, canbuf[i], 3, 16, 0X80, BLUE);        /* ��ʾ���� */
                }
                else 
                {
                    lcd_show_xnum(30 + (i - 4) * 32, 290, canbuf[i], 3, 16, 0X80, BLUE);  /* ��ʾ���� */
                }
            }
        }

        t++; 
        delay_ms(10);

        if (t == 20)
        {
            LED0_TOGGLE();                  /* ��ʾϵͳ�������� */
            t = 0;
            cnt++;
            lcd_show_xnum(30 + 48, 170, cnt, 3, 16, 0X80, BLUE);    /* ��ʾ���� */
        }
    }
}













