/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       RS485 ʵ��
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
#include "./USMART/usmart.h"
#include "./BSP/IIC/myiic.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/RS485/rs485.h"


int main(void)
{
    uint8_t key;
    uint8_t i = 0, t = 0;
    uint8_t cnt = 0;
    uint8_t rs485buf[5]; 

    sys_cache_enable();                       /* ��L1-Cache */
    HAL_Init();                               /* ��ʼ��HAL�� */
    sys_stm32_clock_init(160, 5, 2, 4);       /* ����ʱ��, 400Mhz */
    delay_init(400);                          /* ��ʱ��ʼ�� */
    usart_init(115200);                       /* ���ڳ�ʼ�� */
    usmart_init(200);                         /* ��ʼ��USMART */
    mpu_memory_protection();                  /* ������ش洢���� */
    led_init();                               /* ��ʼ��LED */
    sdram_init();                             /* ��ʼ��SDRAM */
    lcd_init();                               /* ��ʼ��LCD */
    key_init();                               /* ��ʼ��KEY */
    rs485_init(9600);                         /* ��ʼ��RS485 */

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "RS485 TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Send", RED);                /* ��ʾ��ʾ��Ϣ */

    lcd_show_string(30, 150, 200, 16, 16, "Count:", BLUE);
    lcd_show_string(30, 170, 200, 16, 16, "Send Data:", BLUE);
    lcd_show_string(30, 210, 200, 16, 16, "Receive Data:", BLUE);

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)                                               /* KEY0����,����һ������ */
        {
            for (i = 0; i < 5; i++)
            {
                rs485buf[i] = cnt + i;                                      /* ��䷢�ͻ����� */
                lcd_show_num(30 + i * 32, 190, rs485buf[i], 3, 16, 0X80);   /* ��ʾ���� */
            }
            rs485_send_data(rs485buf, 5);       /* ����5���ֽ� */
        }

        rs485_receive_data(rs485buf, &key);

        if (key)                                /* ���յ������� */
        {
            if (key > 5)
            {
                key = 5;                        /* �����5������. */
            }
            for (i = 0; i < key; i++)
            {
                lcd_show_num(30 + i * 32, 230, rs485buf[i], 3, 16, 0X80);   /* ��ʾ���� */
            }
        }

        t++; 
        delay_ms(10);

        if (t == 20)
        {
            LED0_TOGGLE();                      /* ��ʾϵͳ�������� */
            t = 0; 
            cnt++;
            lcd_show_num(30 + 48, 150, cnt, 3, 16, 0X80);                   /* ��ʾ���� */
        }
    }
}
