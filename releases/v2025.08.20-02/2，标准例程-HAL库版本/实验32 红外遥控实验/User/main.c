/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ����ң�� ʵ��
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
#include "./BSP/REMOTE/remote.h"


int main(void)
{
    uint8_t key;
    uint8_t t = 0;
    char *str = 0;

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
    remote_init();                              /* ������ճ�ʼ�� */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "REMOTE TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEYVAL:", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEYCNT:", RED);
    lcd_show_string(30, 150, 200, 16, 16, "SYMBOL:", RED);

    while (1)
    {
        key = remote_scan();

        if (key)
        {
            lcd_show_num(86, 110, key, 3, 16, BLUE);            /* ��ʾ��ֵ */
            lcd_show_num(86, 130, g_remote_cnt, 3, 16, BLUE);   /* ��ʾ�������� */

            switch (key)
            {
                case 0:
                    str = "ERROR";
                    break;

                case 162:
                    str = "POWER";
                    break;

                case 98:
                    str = "UP";
                    break;

                case 2:
                    str = "PLAY";
                    break;

                case 226:
                    str = "ALIENTEK";
                    break;

                case 194:
                    str = "RIGHT";
                    break;

                case 34:
                    str = "LEFT";
                    break;

                case 224:
                    str = "VOL-";
                    break;

                case 168:
                    str = "DOWN";
                    break;

                case 144:
                    str = "VOL+";
                    break;

                case 104:
                    str = "1";
                    break;

                case 152:
                    str = "2";
                    break;

                case 176:
                    str = "3";
                    break;

                case 48:
                    str = "4";
                    break;

                case 24:
                    str = "5";
                    break;

                case 122:
                    str = "6";
                    break;

                case 16:
                    str = "7";
                    break;

                case 56:
                    str = "8";
                    break;

                case 90:
                    str = "9";
                    break;

                case 66:
                    str = "0";
                    break;

                case 82:
                    str = "DELETE";
                    break;
            }

            lcd_fill(86, 150, 116 + 8 * 8, 170 + 16, WHITE);    /* ���֮ǰ����ʾ */
            lcd_show_string(86, 150, 200, 16, 16, str, BLUE);   /* ��ʾSYMBOL */
        }
        else
        {
            delay_ms(10);
        }

        t++;

        if (t == 20)
        {
            t = 0;
            LED0_TOGGLE();                                      /* LED0��˸ */
        }
    }
}


