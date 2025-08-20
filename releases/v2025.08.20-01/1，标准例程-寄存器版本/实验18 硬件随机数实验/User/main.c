/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       RNG(Ӳ�������������) ʵ��
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
#include "./BSP/RNG/rng.h"


int main(void)
{
    uint32_t random;
    uint8_t t = 0, key;
    
    sys_stm32_clock_init(160, 5, 2, 4);     /* ����ʱ��, 400Mhz */
    delay_init(400);                        /* ��ʱ��ʼ�� */
    usart_init(100, 115200);                /* ��ʼ��USART */
    usmart_dev.init(100);                   /* USMART��ʼ�� */
    led_init();                             /* ��ʼ��LED */ 
    mpu_memory_protection();                /* ������ش洢���� */
    sdram_init();                           /* ��ʼ��SDRAM */
    lcd_init();                             /* ��ʼ��LCD */
    key_init();                             /* ��ʼ������ */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "RNG TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

     while (rng_init())                           /* ��ʼ������������� */
    {
        lcd_show_string(30, 110, 200, 16, 16, "  RNG Error! ", RED);
        delay_ms(200);
        lcd_show_string(30, 110, 200, 16, 16, "RNG Trying...", RED);
    }

    lcd_show_string(30, 110, 200, 16, 16, "RNG Ready!   ", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY0:Get Random Num", RED);
    lcd_show_string(30, 150, 200, 16, 16, "Random Num:", RED);
    lcd_show_string(30, 180, 200, 16, 16, "Random Num[0-9]:", RED); 

    while (1)
    {
         key = key_scan(0);

        if (key == KEY0_PRES)
        {
            random = rng_get_random_num();
            lcd_show_num(30 + 8 * 11, 150, random, 10, 16, BLUE);
        }

        if ((t % 20) == 0)
        {
            LED0_TOGGLE();                                      /* ÿ200ms,��תһ��LED0 */
            random = rng_get_random_range(0, 9);                /* ȡ[0,9]���������� */
            lcd_show_num(30 + 8 * 16, 180, random, 1, 16, BLUE);/* ��ʾ����� */
        }

        delay_ms(10);
        t++;
    }
}


