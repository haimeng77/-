/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-22
 * @brief       IO��չʵ�� ʵ��
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
#include "./BSP/PCF8574/pcf8574.h"


int main(void)
{
    uint8_t key;
    uint16_t i = 0;
    uint8_t beepsta = 1;
    
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
    lcd_show_string(30, 70, 200, 16, 16, "PCF8574 TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:BEEP ON/OFF", RED);    /* ��ʾ��ʾ��Ϣ */
    lcd_show_string(30, 130, 200, 16, 16, "EXIO:DS1 ON/OFF", RED);     /* ��ʾ��ʾ��Ϣ */

    while (pcf8574_init()) /* ��ⲻ��PCF8574 */
    {
        lcd_show_string(30, 170, 200, 16, 16, "PCF8574 Check Failed!", RED);
        delay_ms(500);
        lcd_show_string(30, 170, 200, 16, 16, "Please Check!      ", RED);
        delay_ms(500);
        LED0_TOGGLE();     /* �����˸ */
    }

    lcd_show_string(30, 170, 200, 16, 16, "PCF8574 Ready!", RED);

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)                      /* KEY0����,��ȡ�ַ�������ʾ */
        {
            beepsta =! beepsta;                    /* ������״̬ȡ�� */
            pcf8574_write_bit(BEEP_IO, beepsta);   /* ���Ʒ����� */
        }

        if (PCF8574_INT == 0)                      /* PCF8574���жϵ͵�ƽ��Ч */
        {
            key = pcf8574_read_bit(EX_IO);         /* ��ȡEXIO״̬,ͬʱ���PCF8574���ж����(INT�ָ��ߵ�ƽ) */

            if (key == 0)
            {
                LED1_TOGGLE();                     /* LED1״̬ȡ�� */
            }
        }

        i++;
        delay_ms(10);

        if (i == 20)
        {
            LED0_TOGGLE();                        /* �����˸ */
            i = 0;
        }
    }
}











