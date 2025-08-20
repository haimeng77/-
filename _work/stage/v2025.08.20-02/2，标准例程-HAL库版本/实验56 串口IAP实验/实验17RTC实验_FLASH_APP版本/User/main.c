/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       RTC ʵ��
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
#include "./BSP/SDRAM/sdram.h"
#include "./USMART/usmart.h"
#include "./BSP/RTC/rtc.h"
#include "./BSP/MPU/mpu.h"


int main(void)
{
    uint8_t hour, min, sec, ampm;
    uint8_t year, month, date, week;
    uint8_t tbuf[40];
    uint8_t t = 0;

    sys_nvic_set_vector_table(FLASH_BASE, 0x10000);             /* �����ж�����ƫ�� */

    sys_cache_enable();                                         /* ��L1-Cache */
    HAL_Init();                                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);                         /* ����ʱ��, 400Mhz */
    delay_init(400);                                            /* ��ʱ��ʼ�� */
    usart_init(115200);                                         /* ���ڳ�ʼ�� */
    usmart_dev.init(200);                                       /* USMART��ʼ�� */
    mpu_memory_protection();                                    /* ������ش洢���� */
    led_init();                                                 /* ��ʼ��LED */
    sdram_init();                                               /* ��ʼ��SDRAM */
    lcd_init();                                                 /* ��ʼ��LCD */
    rtc_init();                                                 /* ��ʼ��RTC */
    rtc_set_wakeup(RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0);          /* ����WAKE UP�ж�,1�����ж�һ�� */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "RTC TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    while (1)
    {
        t++;

        if ((t % 10) == 0)                                      /* ÿ100ms����һ����ʾ���� */
        {
            rtc_get_time(&hour, &min, &sec, &ampm);
            sprintf((char *)tbuf, "Time:%02d:%02d:%02d", hour, min, sec);
            lcd_show_string(30, 130, 210, 16, 16, (char *)tbuf, RED);
            rtc_get_date(&year, &month, &date, &week);
            sprintf((char *)tbuf, "Date:20%02d-%02d-%02d", year, month, date);
            lcd_show_string(30, 150, 210, 16, 16, (char *)tbuf, RED);
            sprintf((char *)tbuf, "Week:%d", week);
            lcd_show_string(30, 170, 210, 16, 16, (char *)tbuf, RED);
        }

        if ((t % 20) == 0)
        {
            LED0_TOGGLE();                                      /* ÿ200ms,��תһ��LED0 */
        }

        delay_ms(10);
    }
}
