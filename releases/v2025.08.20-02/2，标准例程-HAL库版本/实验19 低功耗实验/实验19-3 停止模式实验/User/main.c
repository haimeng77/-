/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       ֹͣģʽ ʵ��
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
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/PWR/pwr.h"
#include "./BSP/SDRAM/sdram.h"


int main(void)
{
    uint8_t t = 0;
    uint8_t key = 0;

    sys_cache_enable();                    /* ��L1-Cache */
    HAL_Init();                            /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);    /* ����ʱ��, 400Mhz */
    delay_init(400);                       /* ��ʱ��ʼ�� */
    usart_init(115200);                    /* ���ڳ�ʼ�� */
    mpu_memory_protection();               /* ������ش洢���� */
    led_init();                            /* ��ʼ��LED */
    sdram_init();                          /* ��ʼ��SDRAM */
    lcd_init();                            /* ��ʼ��LCD */
    key_init();                            /* ��ʼ������ */
    pwr_wkup_key_init();                   /* ���Ѱ�����ʼ�� */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "STOP TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Enter STOP MODE", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY_UP:Exit STOP MODE", RED);

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)
        {
            LED1(0);                        /* �����̵�,��ʾ����ֹͣģʽ */ 

            pwr_enter_stop();               /* ����ֹͣģʽ */

            /* ��ֹͣģʽ����, ��Ҫ��������ϵͳʱ��, 400Mhz */
            sys_stm32_clock_init(160, 5, 2, 4);
            lcd_init();
            lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
            lcd_show_string(30, 70, 200, 16, 16, "STOP TEST", RED);
            lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
            lcd_show_string(30, 110, 200, 16, 16, "KEY0:Enter STOP MODE", RED);
            lcd_show_string(30, 130, 200, 16, 16, "KEY_UP:Exit STOP MODE", RED);
            LED1(1);                        /* �ر��̵�,��ʾ�˳�ֹͣģʽ */
        }

        if ((t % 20) == 0)
        {
            LED0_TOGGLE();                  /* ÿ200ms,��תһ��LED0 */
        }

        delay_ms(10);
        t++;
    }
}
